#define __STDC_FORMAT_MACROS 1

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <inttypes.h>
#include "cachesim.hpp"

void print_help_and_exit(void) {
    printf("cachesim [OPTIONS] < traces/file.trace\n");
    printf("  -c C\t\tTotal size in bytes is 2^C\n");
    printf("  -b B\t\tSize of each block in bytes is 2^B\n");
    printf("  -s S\t\tNumber of blocks per set is 2^S\n");
    printf("  -v V\t\tNumber of blocks in victim cache\n");
    printf("  -k K\t\tPrefetch Distance");
	printf("  -h\t\tThis helpful output\n");
    exit(0);
}

void print_statistics(cache_stats_t* p_stats);

int main(int argc, char* argv[]) {
    int opt;
    uint64_t c = DEFAULT_C;
    uint64_t b = DEFAULT_B;
    uint64_t s = DEFAULT_S;
    uint64_t v = DEFAULT_V;
    uint64_t k = DEFAULT_K;
    FILE* fin  ;//= fopen("perlbench.trace", "r");

    /* Read arguments */
    while(-1 != (opt = getopt(argc, argv, "c:b:s:i:v:k:h"))) {
        switch(opt) {
        case 'c':
            c = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 's':
            s = atoi(optarg);
            break;
        case 'v':
            v = atoi(optarg);
            break;
		case 'k':
			k = atoi(optarg);
			break;
        case 'i':
            fin = fopen(optarg, "r");
            break;
        case 'h':
            /* Fall through */
        default:
            print_help_and_exit();
            break;
        }
    }

    printf("Cache Settings\n");
    printf("C: %" PRIu64 "\n", c);
    printf("B: %" PRIu64 "\n", b);
    printf("S: %" PRIu64 "\n", s);
    printf("V: %" PRIu64 "\n", v);
	printf("K: %" PRIu64 "\n", k);
    printf("\n");

    /* Setup the cache */
    setup_cache(c, b, s, v, k);

    /* Setup statistics */
    cache_stats_t stats;
    memset(&stats, 0, sizeof(cache_stats_t));

    /* Begin reading the file */
    char rw;
    uint64_t address;
    while (!feof(fin)) {
        int ret = fscanf(fin, "%c %" PRIx64 "\n", &rw, &address);
        if(ret == 2) {
            cache_access(rw, address, &stats);
        }
    }

    complete_cache(&stats, b, s);

    print_statistics(&stats);

    return 0;
}

void print_statistics(cache_stats_t* p_stats) {
    printf("Cache Statistics\n");
    printf("Accesses: %" PRIu64 "\n", p_stats->accesses);
    printf("Reads: %" PRIu64 "\n", p_stats->reads);
    printf("Read misses: %" PRIu64 "\n", p_stats->read_misses);
    printf("Read misses combined: %" PRIu64 "\n", p_stats->read_misses_combined);
    printf("Writes: %" PRIu64 "\n", p_stats->writes);
    printf("Write misses: %" PRIu64 "\n", p_stats->write_misses);
    printf("Write misses combined: %" PRIu64 "\n", p_stats->write_misses_combined);
    printf("Misses: %" PRIu64 "\n", p_stats->misses);
    printf("Writebacks: %" PRIu64 "\n", p_stats->write_backs);
	printf("Victim cache misses: %" PRIu64 "\n", p_stats->vc_misses);
	printf("Prefetched blocks: %" PRIu64 "\n", p_stats->prefetched_blocks);
	printf("Useful prefetches: %" PRIu64 "\n", p_stats->useful_prefetches);
	printf("Bytes transferred to/from memory: %" PRIu64 "\n", p_stats->bytes_transferred);
	printf("Hit Time: %f \n", p_stats->hit_time);
    printf("Miss Penalty: %" PRIu64 " \n", p_stats->miss_penalty);
    printf("Miss rate: %f \n", p_stats->miss_rate);
    printf("Average access time (AAT): %f\n", p_stats->avg_access_time);
}

