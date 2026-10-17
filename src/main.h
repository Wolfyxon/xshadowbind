#pragma once

#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "x11.h"

int main(int agrc, char **argv);

void parse_args(AppContext *ctx, int argc, char **argv);

void print_help();
void check_key(AppContext *ctx, const char *key_name);
void add_bind(AppContext *ctx, const char *combo, const char *command);

void main_loop(AppContext *ctx);
