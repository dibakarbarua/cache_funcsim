#ifndef CACHESIM_HPP
#define CACHESIM_HPP

//#include <cinttypes>
#include <stdint.h>

struct cache_stats_t {
    uint64_t accesses;
    uint64_t reads;
    uint64_t read_misses;
    uint64_t read_misses_combined;
    uint64_t writes;
    uint64_t write_misses;
    uint64_t write_misses_combined;
    uint64_t misses;
	uint64_t write_backs;
	uint64_t vc_misses;
	uint64_t prefetched_blocks;
	uint64_t useful_prefetches;
	uint64_t bytes_transferred;

	//update in COMPLETE_CACHE
    double hit_time;
    uint64_t miss_penalty;
    double   miss_rate;
    double   avg_access_time;
};

void cache_access(char rw, uint64_t address, cache_stats_t* p_stats);
void setup_cache(uint64_t c, uint64_t b, uint64_t s, uint64_t v, uint64_t k);
void complete_cache(cache_stats_t *p_stats, uint64_t b, uint64_t s);

static const uint64_t DEFAULT_C = 15;   /* 32KB Cache */
static const uint64_t DEFAULT_B = 5;    /* 32-byte blocks */
static const uint64_t DEFAULT_S = 3;    /* 8 blocks per set */
static const uint64_t DEFAULT_V = 4;    /* 4 victim blocks */
static const uint64_t DEFAULT_K = 2;	/* 2 prefetch distance */

/** Argument to cache_access rw. Indicates a load */
static const char     READ = 'r';
/** Argument to cache_access rw. Indicates a store */
static const char     WRITE = 'w';

#endif /* CACHESIM_HPP */
