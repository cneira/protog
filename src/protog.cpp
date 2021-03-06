#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/compiler/parser.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "parser.h"
#include "yajl_writer.h"

static const char* DEFAULT_OUTPUT_DIR = ".";

void print_help(FILE* f) {
    fprintf(f, "Usage: protog [OPTIONS]\n");
    fprintf(f, "Parse given proto file and generate output based on the options given:\n");
    fprintf(f, "  -h                 Print this help message.\n");
    fprintf(f, "  -d                 Enable debug output.\n");
    fprintf(f, "  -p PROTO_FILE      The protobuf file containing the desired proto message.\n");
    fprintf(f, "  -m PROTO_MESSAGE   Fully qualified name of the message for which the\n");
    fprintf(f, "                     parser code should be generated.\n");
    fprintf(f, "  -i PROTO_INCLUDE   Name of the header file generated by protoc.\n");
    fprintf(f, "  -o OUTPUT_DIR      Folder where generated source files should be placed\n");
    fprintf(f, "                     It defaults to \"%s\".\n", DEFAULT_OUTPUT_DIR);
    fprintf(f, "Example usage:\n");
    fprintf(f, "  protog -p openrtb.proto -m com.google.openrtb.BidRequest -i openrtb.pb.h\n");
}

int main(int argc, char **argv) {
    bool debug = false;
    const char* output_dir = DEFAULT_OUTPUT_DIR;
    // TODO: derive proto header name from proto_file
    const char* proto_include = NULL;
    const char* proto_message = NULL;
    const char* proto_file = NULL;

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "hdo:i:m:p:")) != -1) {
        switch (c) {
        case 'h':
            print_help(stdout);
            exit(EXIT_SUCCESS);
        case 'd':
            debug = true;
            break;
        case 'p':
            proto_file = optarg;
            break;
        case 'm':
            proto_message = optarg;
            break;
        case 'i':
            proto_include = optarg;
            break;
        case 'o':
            output_dir = optarg;
            break;
        default:
            print_help(stderr);
            exit(EXIT_FAILURE);
        }
    }

    if (!proto_file || !proto_message || !proto_include || !output_dir) {
        fprintf(stderr, "Missing required argument.\n");
        print_help(stderr);
        exit(EXIT_FAILURE);
    }

    protog::Graph graph{proto_file, proto_message};
    graph.parseMessageDesc();
    if (debug) {
        graph.printDebug(stdout);
    }

    auto writer = std::make_shared<protog::YajlWriter>();
    writer->write(graph, proto_include);

    return 0;
}
