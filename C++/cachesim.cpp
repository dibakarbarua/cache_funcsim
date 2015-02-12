/*DIBAKAR BARUA CACHE SIMULATOR */
#include "cachesim.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include<algorithm>
#include<stdint.h>
#include<math.h>
#include <stdio.h>
#include <stdlib.h>
//#include<conio.h>
#include<iostream>
#include<iomanip>

using namespace std;


class BlockNode{

    public:
    uint64_t addr; //change this to 64 for actual
    bool valid;
    bool prefetched;
    bool dirty;
    uint64_t time;
    BlockNode()
	{
    	valid = false;
    	time = 0;
	}
};

//global variables
uint64_t* CountSets;
BlockNode* Cache; //main cache
BlockNode* vc;  //victim cache
uint64_t num_of_sets;
uint64_t blocks_per_set;
uint64_t block_size;
uint64_t cache_size;
int offset_bits;
int index_bits;
int vc_count;
int vc_size;
//prefetcher

uint64_t last_miss_addr;
uint64_t pending_stride;
uint64_t count_p = 0;
//uint64_t degree_prefetch;
int degree_prefetch;

//Cache stats
/*
uint64_t num_of_prefetches;
uint64_t useful_prefetches = 0;
uint64_t accesses = 0;
uint64_t reads = 0;
uint64_t read_misses = 0;
uint64_t read_misses_vc = 0;
uint64_t writes = 0;
uint64_t write_misses = 0;
uint64_t write_misses_vc = 0;
uint64_t num_of_writebacks = 0;
uint64_t misses = 0;
uint64_t misses_vc = 0;*/


void setup_cache(uint64_t c, uint64_t b, uint64_t s, uint64_t v, uint64_t k) {
        //cout<<"SETTING UP THE CACHE"<<endl<<endl;
        num_of_sets = pow(2,c-b-s);
        blocks_per_set = pow(2,s);
        block_size = pow(2,b);
        cache_size = pow(2,c);
        offset_bits = b;
        index_bits = c-b-s;
        vc_count = 0;
        vc_size = v;
        degree_prefetch = k;

        uint64_t i;
        CountSets = new uint64_t[num_of_sets];
        for(i=0; i<num_of_sets; i++)
        {
        	CountSets[i] = 0;
        }
        Cache = new BlockNode[cache_size];
		for(i=0; i<cache_size; i++)
		{
				Cache[i].addr = 0;
				Cache[i].valid = false;
				Cache[i].time = 0;
				Cache[i].prefetched = false;
				Cache[i].dirty = false;
		}
		vc = new BlockNode[v];
        for(i=0; i<v; i++)
		{
				vc[i].addr = 0;
				vc[i].valid = false;
				vc[i].time = 0;
				vc[i].prefetched = false;
				vc[i].dirty = false;
		}
		//cout<<"DONE SETTING UP THE CACHE"<<endl<<endl;
}

uint64_t block_addr(uint64_t X)
{
	 	uint64_t mul;
		mul = (pow(2,offset_bits)-1);
		mul = ~mul;
		X = X & mul;
		return X;
}

void prefetch(uint64_t address, cache_stats_t* p_stats)
{
        //printf("\n\nPREFETCH!!!\n\n");
        p_stats->prefetched_blocks++;
        uint64_t mul, temp; //change to 64 later
		uint64_t index;
		uint64_t d; //for pending stride
		uint64_t addr_o = address;
		mul = (pow(2,offset_bits)-1);
		mul = ~mul;
		address = address & mul;
		mul = pow(2,index_bits)-1;
		temp = address >> offset_bits;
		index = mul & temp;
		//printf("block address is %x and index is %x\n", address, index);
		uint64_t myPos = index * blocks_per_set;
		uint64_t myCount = CountSets[index];

        uint64_t i,j;
		BlockNode temp2;
		bool flag_hit = false;
		for(i=0;i<blocks_per_set;i++)
			{
				Cache[myPos + i].time++;
			}
        uint64_t max_time = Cache[myPos].time;
        for(i=0;i<blocks_per_set;i++)
			{
				if(Cache[myPos + i].time > max_time)
                    max_time = Cache[myPos + i].time;
			}
		if(myCount < blocks_per_set)
		{
			for(i=0;i<myCount;i++)
			{
				uint64_t addr_cmp = block_addr(Cache[myPos + i].addr);

				if(address == addr_cmp)
				{
					//printf("Found Prefetched block %x at %d\n\n", addr_o,i);
					flag_hit = true;

					return;

				}
			}

			if(!flag_hit)
			{

				//printf("SUCCESSFUL PREFETCH at %d\n",myPos + i);
				Cache[myPos + myCount].addr = address;
				Cache[myPos + myCount].time = max_time + 1;
				Cache[myPos + myCount].valid = true;
				Cache[myPos + myCount].prefetched = true;
				Cache[myPos + myCount].dirty = false;
				CountSets[index]++;
			}
			//printf("Set Count is %d\n\n", (CountSets[myPos]));

		}
		else
		{
			for(i=0;i<blocks_per_set;i++)
			{
				uint64_t addr_cmp = block_addr(Cache[myPos + i].addr);

				if(address == addr_cmp)
				{
					//printf("Found Prefetched block %x at %d\n\n", addr_o,i);
                    flag_hit = true;
					return;
				}
			}

			if(!flag_hit)
            {
                //printf("SUCCESSFUL PREFETCH at %d\n",myPos);
                //printf("LRU Element REPLACED was %x\n", Cache[myPos].addr);
                flag_hit = false;
                //printf("SEARCHING in VICTIM CACHE\n");
				for(i=0;i<vc_count;i++)
                    {
                        uint64_t addr_cmp = block_addr(vc[i].addr);

                        if(address == addr_cmp)
                        {
                            //printf("RESULT:%x  			HIT in VICTIM CACHE at %d\n\n", addr_o,i);
                            BlockNode temp4;
                            temp4 = Cache[myPos];
                            Cache[myPos] = vc[i];
                            Cache[myPos].time = max_time + 1;
                            Cache[myPos].prefetched = true;
                            vc[i] = temp4;
                            vc[i].time = 0;
                            flag_hit = true;
                            break;
                        }

                    }

                if(!flag_hit)
                {
                //printf("SUCCESSFUL PREFETCH at %d\n",myPos);
                //printf("LRU Element REPLACED was %x\n", Cache[myPos].addr);
                if(vc_count < vc_size)
                        {
                                temp = Cache[myPos].addr;
                                bool temp3,temp4;
                                temp3 = Cache[myPos].prefetched;
                                temp4 = Cache[myPos].dirty;
                                Cache[myPos].addr = address;
                                Cache[myPos].prefetched = true;
                                Cache[myPos].dirty = false;
                                Cache[myPos].time = max_time + 1;
                                vc[vc_count].addr = temp;
                                vc[vc_count].valid = true;
                                vc[vc_count].prefetched = temp3;
                                vc[vc_count].dirty = temp4;
                                vc_count++;
                        }

                        else if( vc_size > 0)
                        {
                                //FIFO Replacement
                                if(vc[0].dirty)
                                {
                                    p_stats->write_backs++;
                                    vc[0].dirty = false;
                                }

                                for(i=0;i<vc_size-1;i++)
                                {
                                        vc[i] = vc[i+1];
                                }
                                vc[vc_size-1] = Cache[myPos];
                                Cache[myPos].addr = address;
                                Cache[myPos].time = max_time + 1;
                                Cache[myPos].prefetched = true;
                                Cache[myPos].dirty = false;

                        }

                        else{
                            //printf("INSERTING at %d\n",myPos);
                            //printf("LRU Element REPLACED was %x\n", Cache[myPos].addr);
                            if(Cache[myPos].dirty)
                            {
                                p_stats->write_backs++;
                                Cache[myPos].dirty = false;
                            }
                            Cache[myPos].addr = address;
                            Cache[myPos].time = max_time + 1;
                            Cache[myPos].valid = true;
                            Cache[myPos].prefetched = true;
                            Cache[myPos].dirty = false;
                        }


            }
            }
		}

		for (i = 0; i < (CountSets[index]); i++){
		j = i;

		while (j > 0 && Cache[myPos + j].time > Cache[myPos + j - 1].time){
			  temp2 = Cache[myPos + j];
			  Cache[myPos + j] = Cache[myPos + j - 1];
			  Cache[myPos+j-1] = temp2;
			  j--;
			  }
		}

		for (i = 0; i < 4; i++){
			//printf("CURRENT STATUS: At location %d element is %x timestamp is %d\n", myPos + i, Cache[myPos + i].addr, Cache[myPos + i].time);
            //cout<<"CURRENT STATUS: location "<<myPos + i<<"  "<<hex<<Cache[myPos + i].addr<<" time is "<<Cache[myPos + i].time<<" prefetched is "<<Cache[myPos + i].prefetched<<" dirty is "<<Cache[myPos + i].dirty<<endl;
		}

        for (i = 0; i < vc_size; i++){
		//cout<<"VICTIM CACHE: "<<i<<" is "<<hex<<vc[i].addr<<" prefetched is "<<vc[i].prefetched<<" dirty is "<<vc[i].dirty<<endl;
        }

        return;
}

void cache_access(char rw,uint64_t address, cache_stats_t* p_stats) {
		//printf("THERE ARE %d ELEMENTS\n", cache_size);
		p_stats->accesses++;
		if(rw == 'r')
            p_stats->reads++;
        else if(rw == 'w')
            p_stats->writes++;

		uint64_t mul, temp; //change to 64 later
		uint64_t index;
		uint64_t d; //for pending stride
		uint64_t addr_o = address;
		mul = (pow(2,offset_bits)-1);
		mul = ~mul;
		address = address & mul;
		mul = pow(2,index_bits)-1;
		temp = address >> offset_bits;
		index = mul & temp;
		//printf("block address is %x and index is %x\n", address, index);
		uint64_t myPos = index * blocks_per_set;
		//CODE
		//printf("myPos = %d\n", myPos);
		uint64_t myCount = CountSets[index];
		//printf("myCount = %d\n", myCount);
		uint64_t i,j;
		BlockNode temp2;
		bool flag_hit = false;
		for(i=0;i<blocks_per_set;i++)
			{
				Cache[myPos + i].time++;
			}
		if(myCount < blocks_per_set)
		{
			//printf("BLOCKS LEFT\n");
			//printf("SEARCHING at %d\n", myPos);
			for(i=0;i<myCount;i++)
			{
				uint64_t addr_cmp = block_addr(Cache[myPos + i].addr);

				if(address == addr_cmp)
				{
					//printf("RESULT: %x  			hit at %d\n\n", addr_o,i);
					Cache[myPos + i].time = 0;
					Cache[myPos + i].addr = address;
					if(Cache[myPos + i].prefetched)
                    {
                        p_stats->useful_prefetches++;
                        Cache[myPos + i].prefetched = false;

                    }
                    if(rw == 'w')
                        Cache[myPos + i].dirty = true;
                    //else
                    //    Cache[myPos + i].dirty = false;

					flag_hit = true;
				}
			}

			if(!flag_hit)
			{
				if(rw == 'r')
                 {
                    p_stats->read_misses++;
                    p_stats->read_misses_combined++;
                 }
                else if(rw == 'w')
                {
                    p_stats->write_misses++;
                    p_stats->write_misses_combined++;
                }

				//printf("NOT FOUND\n");
				//printf("INSERTING at %d\n",myPos + i);
				Cache[myPos + myCount].addr = address;
				Cache[myPos + myCount].time = 0;
				Cache[myPos + myCount].valid = true;
				Cache[myPos + myCount].prefetched = false;
				Cache[myPos + myCount].dirty = false;
				if(rw == 'w')
                    Cache[myPos + myCount].dirty = true;
                //else
                //    Cache[myPos + myCount].dirty = false;
				CountSets[index]++;

				//sort before prefetch
				for (i = 0; i < (CountSets[index]); i++){
                j = i;

                while (j > 0 && Cache[myPos + j].time > Cache[myPos + j - 1].time){
                temp2 = Cache[myPos + j];
                Cache[myPos + j] = Cache[myPos + j - 1];
                Cache[myPos+j-1] = temp2;
                j--;
                    }
                }


				//prefetch
				count_p++;
				d = address - last_miss_addr;
				if(count_p >= 3)
                {
                    if(d == pending_stride)
                    {
                        for(i=1;i<=degree_prefetch;i++)
                        {
                            prefetch(address + i*d, p_stats);
                            //printf("PREFETCHED WITH EMPTY\n\n");
                        }
                    }

                }
                last_miss_addr = address;
                pending_stride = d;

				//printf("RESULT:%x  			miss with empty\n\n", addr_o);
			}
			//printf("Set Count is %d\n\n", (CountSets[myPos]));

		}
		else
		{
			//printf("SETS FULL\n");
			//printf("SEARCHING at %d\n", myPos);
			for(i=0;i<blocks_per_set;i++)
			{
				uint64_t addr_cmp = block_addr(Cache[myPos + i].addr);

				if(address == addr_cmp)
				{
					//printf("RESULT:%x  			hit at %d\n\n", addr_o,i);
					Cache[myPos + i].time = 0;
					Cache[myPos + i].addr = address;
					if(Cache[myPos + i].prefetched)
                    {
                        p_stats->useful_prefetches++;
                        Cache[myPos + i].prefetched = false;

                    }
                    if(rw == 'w')
                        Cache[myPos + i].dirty = true;
                    //else
                    //    Cache[myPos + i].dirty = false;
					flag_hit = true;
				}
			}

			if(!flag_hit)
			{
				/*printf("INSERTING at %d\n",myPos);
				printf("LRU Element REPLACED was %x\n", Cache[myPos].addr);
				Cache[myPos].addr = address;
				Cache[myPos].time = 0;
				Cache[myPos].valid = true;*/
				if(rw == 'r')
                    p_stats->read_misses++;
                else if(rw == 'w')
                    p_stats->write_misses++;
				//printf("RESULT:%x  			miss in MAIN CACHE\n\n", addr_o);
				//VC ACCESS
				flag_hit = false;
                //printf("SEARCHING in VICTIM CACHE\n");
				for(i=0;i<vc_count;i++)
                    {
                        uint64_t addr_cmp = block_addr(vc[i].addr);

                        if(address == addr_cmp)
                        {
                            //printf("RESULT:%x  			HIT in VICTIM CACHE at %d\n\n", addr_o,i);
                            bool temp2;
                            if(vc[i].prefetched)
                            {
                                p_stats->useful_prefetches++;
                                vc[i].prefetched = false;

                            }
                            if(rw == 'w')
                                vc[i].dirty = true;
                            //else
                            //    vc[i].dirty = false;
                            Cache[myPos].time = 0;
                            temp = Cache[myPos].addr;
                            Cache[myPos].addr = address;
                            vc[i].addr = temp;
                            //swapping prefetched bool
                            temp2 = Cache[myPos].prefetched;
                            Cache[myPos].prefetched = vc[i].prefetched;
                            vc[i].prefetched = temp2;
                            //swapping dirty bool
                            temp2 = Cache[myPos].dirty;
                            Cache[myPos].dirty = vc[i].dirty;
                            vc[i].dirty = temp2;
                            flag_hit = true;
                            break;
                        }

                    }
                if(!flag_hit)
                    {
                        //printf("RESULT:%x  			MISS in VICTIM CACHE\n\n", addr_o);
                        if(rw == 'r')
                            p_stats->read_misses_combined++;
                        else if(rw == 'w')
                            p_stats->write_misses_combined++;
                        if(vc_count < vc_size)
                        {
                                temp = Cache[myPos].addr;
                                vc[vc_count].prefetched = Cache[myPos].prefetched;
                                vc[vc_count].dirty = Cache[myPos].dirty;
                                Cache[myPos].addr = address;
                                Cache[myPos].time = 0;
                                Cache[myPos].prefetched = false;
                                Cache[myPos].dirty = false;
                                if(rw == 'w')
                                    Cache[myPos].dirty = true;
                                //else
                                //   Cache[myPos].dirty = false;
                                vc[vc_count].addr = temp;
                                vc[vc_count].valid = true;
                                vc_count++;
                        }

                        else if( vc_size > 0)
                        {
                                //FIFO Replacement
                                if(vc[0].dirty)
                                {
                                    p_stats->write_backs++;
                                    vc[0].dirty = false;
                                }

                                for(i=0;i<vc_size-1;i++)
                                {
                                        vc[i] = vc[i+1];
                                }
                                vc[vc_size-1] = Cache[myPos];
                                Cache[myPos].addr = address;
                                Cache[myPos].time = 0;
                                Cache[myPos].prefetched = false;
                                Cache[myPos].dirty = false;
                                if(rw == 'w')
                                    Cache[myPos].dirty = true;
                                //else
                                //    Cache[myPos].dirty = false;

                        }

                        else{
                            //printf("INSERTING at %d\n",myPos);
                            //printf("LRU Element REPLACED was %x\n", Cache[myPos].addr);
                            if(Cache[myPos].dirty)
                            {
                                p_stats->write_backs++;
                                Cache[myPos].dirty = false;
                            }

                            Cache[myPos].addr = address;
                            Cache[myPos].time = 0;
                            Cache[myPos].valid = true;
                            Cache[myPos].prefetched = false;
                            Cache[myPos].dirty = false;
                            if(rw == 'w')
                                Cache[myPos].dirty = true;
                            //else
                            //    Cache[myPos].dirty = false;
                        }

                        for (i = 0; i < (CountSets[index]); i++){
                        j = i;

                        while (j > 0 && Cache[myPos + j].time > Cache[myPos + j - 1].time){
                        temp2 = Cache[myPos + j];
                        Cache[myPos + j] = Cache[myPos + j - 1];
                        Cache[myPos+j-1] = temp2;
                        j--;
                            }
                        }

                    }
                        count_p++;
                        d = address - last_miss_addr;
                        if(count_p >= 3)
                        {
                            if(d == pending_stride)
                            {
                                for(i=1;i<=degree_prefetch;i++)
                                {
                                    prefetch(address + i*d, p_stats);
                                    //printf("PREFETCHED WITHOUT EMPTY\n\n");
                                }
                            }

                        }
                        last_miss_addr = address;
                        pending_stride = d;





                }


			}


		for (i = 0; i < (CountSets[index]); i++){
		j = i;

		while (j > 0 && Cache[myPos + j].time > Cache[myPos + j - 1].time){
			  temp2 = Cache[myPos + j];
			  Cache[myPos + j] = Cache[myPos + j - 1];
			  Cache[myPos+j-1] = temp2;
			  j--;
			  }
		}

		for (i = 0; i < 4; i++){
			//printf("CURRENT STATUS: At location %d element is %x timestamp is %d\n", myPos + i, Cache[myPos + i].addr, Cache[myPos + i].time);
            //cout<<"CURRENT STATUS: location "<<myPos + i<<"  "<<hex<<Cache[myPos + i].addr<<" time is "<<Cache[myPos + i].time<<" prefetched is "<<Cache[myPos + i].prefetched<<" dirty is "<<Cache[myPos + i].dirty<<endl;
		}

        for (i = 0; i < vc_size; i++){
		//cout<<"VICTIM CACHE: "<<i<<" is "<<hex<<vc[i].addr<<" prefetched is "<<vc[i].prefetched<<" dirty is "<<vc[i].dirty<<endl;
        }
}


/**
 * Subroutine for cleaning up any outstanding memory operations and calculating overall statistics
 * such as miss rate or average access time.
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
void complete_cache(cache_stats_t *p_stats, uint64_t b, uint64_t s) {
    p_stats->misses = p_stats->read_misses + p_stats->write_misses;
    p_stats->vc_misses = p_stats->read_misses_combined + p_stats->write_misses_combined;
    p_stats->hit_time = (double)(2 + 0.2*s);
    p_stats->miss_penalty = 200;
    p_stats->miss_rate = ((double) p_stats->misses / (double)p_stats->accesses);
    p_stats->avg_access_time = p_stats->hit_time + ((double) p_stats->vc_misses / (double)p_stats->accesses) * p_stats->miss_penalty;
    p_stats->bytes_transferred = pow(2,b) * (p_stats->vc_misses + p_stats->write_backs + p_stats->prefetched_blocks);
}
