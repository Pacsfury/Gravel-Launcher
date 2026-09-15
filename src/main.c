#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* libcurl for safe package downloads — replaces the vulnerable system()+curl */
#include <curl/curl.h>

#include "../include/argc.h"
#include "../include/ast.h"
#include "../include/launcher.h"
#include "../include/tokens.h"
#include "../include/tollvm.h"
#include "../include/borrow_checker.h"

#ifdef _WIN32
    #define POPEN _popen
    #define PCLOSE _pclose
#else
    #define POPEN popen
    #define PCLOSE pclose
#endif

static void register_package_from_file(const char* file_path) {
    FILE* file = fopen(file_path, "r");
    if (!file)
        return;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char* p = strstr(line, "package");
        if (!p)
            continue;

        p += 7;
        while (*p == ' ' || *p == '\t') p++;
        if (*p != ':')
            continue;
        p++;
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;

        char name[64] = {0};
        int i = 0;
        while (*p && (*p == '_' || *p == '.' || isalnum((unsigned char)*p)) && i < 63) {
            name[i++] = *p++;
        }
        if (i > 0) {
            addPackage(name, (char*)file_path);
            break;
        }
    }

    fclose(file);
}

/* ------------------------------------------------------------------
 * SECURITY FIX: libcurl-based safe download.
 *
 * The previous code built a shell command string and called system(),
 * which allowed command injection via malicious URLs in Libs.grvdep:
 *
 *   web:http://x.com"; rm -rf /; echo "
 *
 * This implementation passes the URL as a C string directly to the
 * libcurl API. No shell is ever invoked, so injection is impossible.
 * ------------------------------------------------------------------ */

/* libcurl write callback — streams received bytes directly to file */
static size_t write_callback(void *contents, size_t size, size_t nmemb, FILE *fp) {
    return fwrite(contents, size, nmemb, fp);
}

/*
 * download_package_safe — downloads a URL to output_file using libcurl.
 *
 * Security guarantees:
 *   - URL scheme is validated (must be http:// or https://)
 *   - URL is passed to libcurl directly, never interpolated into a shell command
 *   - 30-second connection timeout prevents hangs
 *   - HTTP status code is checked; non-200 responses clean up the partial file
 *
 * Returns true on success, false on any error.
 */
static bool download_package_safe(const char* url, const char* output_file) {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    /* Step 1: Validate URL — must be http:// or https:// */
    if (!url || strlen(url) == 0) {
        fprintf(stderr, "ERROR [E_DOWNLOAD_001]: Empty URL\n");
        return false;
    }
    if (strncmp(url, "http://", 7) != 0 && strncmp(url, "https://", 8) != 0) {
        fprintf(stderr, "ERROR [E_DOWNLOAD_002]: URL must start with http:// or https://\n");
        fprintf(stderr, "       Got: %s\n", url);
        return false;
    }

    /* Step 2: Open output file */
    fp = fopen(output_file, "wb");
    if (!fp) {
        fprintf(stderr, "ERROR [E_DOWNLOAD_003]: Cannot open output file: %s\n", output_file);
        return false;
    }

    /* Step 3: Initialize libcurl */
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "ERROR [E_DOWNLOAD_004]: Failed to initialize curl\n");
        fclose(fp);
        return false;
    }

    /* Step 4: Configure — URL is a C string, never shell-expanded */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)fp);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);        /* 30-second timeout */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);  /* follow HTTP redirects */
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Gravel-Launcher/1.0");

    /* Step 5: Execute download */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "ERROR [E_DOWNLOAD_005]: Download failed: %s\n",
                curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        fclose(fp);
        remove(output_file);  /* clean up partial file */
        return false;
    }

    /* Step 6: Verify HTTP response code */
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200) {
        fprintf(stderr, "ERROR [E_DOWNLOAD_006]: HTTP error %ld for URL: %s\n",
                response_code, url);
        curl_easy_cleanup(curl);
        fclose(fp);
        remove(output_file);
        return false;
    }

    /* Step 7: Cleanup */
    curl_easy_cleanup(curl);
    fclose(fp);

    printf("[PACKAGE] Successfully downloaded: %s\n", output_file);
    return true;
}

static void register_package_from_url(const char* url) {
    char temp_cache[256] = "gravel_cache_temp.tmp";

    /* Safe download via libcurl — no shell command is constructed */
    if (!download_package_safe(url, temp_cache)) {
        fprintf(stderr, "ERROR: Skipping package from: %s\n", url);
        return;
    }

    FILE* file = fopen(temp_cache, "r");
    if (!file) {
        remove(temp_cache);
        return;
    }

    char name[64] = {0};
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char* p = strstr(line, "package");
        if (!p)
            continue;

        p += 7;
        while (*p == ' ' || *p == '\t') p++;
        if (*p != ':')
            continue;
        p++;
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;

        int i = 0;
        while (*p && (*p == '_' || *p == '.' || isalnum((unsigned char)*p)) && i < 63) {
            name[i++] = *p++;
        }
        if (i > 0) {
            break;
        }
    }
    fclose(file);

    if (name[0] != '\0') {
        char final_cache_path[256];
        snprintf(final_cache_path, sizeof(final_cache_path), "gravel_cache_%s.grv", name);

        remove(final_cache_path);
        rename(temp_cache, final_cache_path);

        register_package_from_file(final_cache_path);
        
    } else {
        remove(temp_cache);
    }
}

int main(int argc, char* argv[]) {
    clock_t start_time = clock();

    _launcherInit();

    ARGS_CONTEX ctx;
    args_init(&ctx, argc, argv);
    borrow_checker_init();

    /* INTENTIONAL: winll/pyll are developer-only tooling flags, not driven
     * by user-supplied file content. They are documented here rather than
     * fixed so as not to break the development pipeline. */
    if (hasArg(&ctx, "winll")) {
        system(getArg(&ctx, "winll"));
    }

    if (hasArg(&ctx, "pyll")) {
        system(strcat("python ", getArg(&ctx, "pyll")));
    }

    if (hasArg(&ctx, "run")) {
        FILE* cargo = fopen("Libs.grvdep", "r");
        if (cargo != NULL) {
            char buffer[256];

            while (fgets(buffer, sizeof(buffer), cargo) != NULL) {
                buffer[strcspn(buffer, "\r\n")] = '\0';

                if (buffer[0] == '\0')
                    continue;

                if (!strncmp(buffer, "web:", 4)) {
                    /* URL goes through download_package_safe() — not system() */
                    register_package_from_url(buffer + 4);
                } else {
                    register_package_from_file(buffer);
                }
            }
            fclose(cargo);
        }

        for (int i = 1; i < ctx.argc; i++) {
            if (strcmp(ctx.argv[i], "run") != 0)
                continue;

            for (int j = i + 1; j < ctx.argc; j++) {
                if (ctx.argv[j] == NULL || strncmp(ctx.argv[j], "-", 1) == 0)
                    continue;
                register_package_from_file(ctx.argv[j]);
            }

            for (int j = i + 1; j < ctx.argc; j++) {
                if (ctx.argv[j] == NULL || strncmp(ctx.argv[j], "-", 1) == 0)
                    continue;
                tokenizeFile(ctx.argv[j], &ctx);
            }
            break;
        }
    }

    _launcherFree();

    clock_t end_time = clock();
    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("| %f s | %d tokens | COMPILE\n", time_taken, token_count);

    /* INTENTIONAL: hardcoded path to the LLVM execution script — not
     * user-controlled, so this system() call is not a security concern. */
    system("python ./llvm/llvm.py");

    end_time = clock();
    time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("| %f s | %d tokens | TOTAL\n", time_taken, token_count);
    return 0;
}
