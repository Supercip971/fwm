#pragma once
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

namespace feather
{

    constexpr unsigned int version = 0;
    constexpr unsigned int subversion = 1;
    class gcontext
    {
        FILE *output = nullptr;

    public:
        bool error = false;
        const char *last_error;
        void generate_error(const char *error_name, ...)
        {
            va_list vl;
            va_start(vl, error_name);
            error = true;

            char *target = new char[strlen(error_name) + 20];
            vsprintf(target, error_name, vl);
            fprintf(stderr, "\033[1;31m[ ERROR ]\033[0m %s \n", target);
            va_end(vl);
            delete[] target;
        }

        void log(const char *message, ...)
        {
            if (output == nullptr)
            {
                output = fopen("fwm_output", "w");
            }
            va_list vl;
            va_start(vl, message);

            error = true;

            char *target = new char[strlen(message) + 128];
            vsprintf(target, message, vl);

            fprintf(output, "\033[1;36m[ FWM ]\033[0m %s  \n",
                    target);
            printf("\033[1;36m[ FWM ]\033[0m %s  \n",
                   target);

            va_end(vl);
            delete[] target;
        }
        void close()
        {
            fclose(output);
        }
    };

    static gcontext context;
} // namespace feather