# import redis
import string
import redis.asyncio as redis
import sys
from core.engine_utils import convert_bytes_list_to_response
from core.prefix_processor import PrefixProcessor
# https://www.dragonflydb.io/code-examples/python-redis-zadd
from enum import IntEnum

class Prefix_Change(IntEnum):
    Modified = 0
    Added = 1

class MemoryCache:
    def __init__(self):
        self.cache = redis.Redis(host='localhost', port=6379, db=0)

    async def flush_db(self):
        await self.cache.flushdb() # Delete all keys of currently selected database instance

    async def flush_all(self):
        await self.cache.flushall() # Delete all keys of entire database

    async def add_pair(self, name: string, mapping_key: string, mapping_value:int):
        await self.cache.zadd(name, {mapping_key: mapping_value})

    async def add_data(self, name: string, data: dict):
        await self.cache.zadd(name, data)

    async def get_top_ranked_prefixes(self, name: string, start_index: int = 0, end_index: int = -1, with_scores:bool=False ) -> list[dict]:
        res = await self.cache.zrevrange(name, start_index, end_index, withscores=with_scores)
        if with_scores:
            return res
        return convert_bytes_list_to_response(res)

    async def delete_least_ranked_prefix(self, name: string, count: int=1) -> list[tuple]:
        return await self.cache.zpopmin(name, count)

    async def add_value_to_completion(self, name: string, prefix: string, value: float=1.0):
        return await self.cache.zincrby(name, value, prefix)  # Increment the score of value in sorted set name by amount

    async def remove_element(self, name: string, completion:string ):
        return await self.cache.zrem(name, completion)

    # https: // www.tutorialspoint.com / redis / sorted_sets_zincrby.htm
    # https: // medium.com / analytics - vidhya / redis - sorted - sets - explained - 2d8b6302525
    async def update_completion_counter(self, completion: string, max_completions = 5) -> Prefix_Change:
        length_limit = 30
        prefix_added = False
        for i in range(min(len(completion),length_limit)):
            prefix = completion[ : i+1]

            res = await self.get_top_ranked_prefixes(prefix, 0, max_completions-1, with_scores=True)
            value = res[-1][1]+1.0
            await self.add_value_to_completion(prefix,completion, value)
            res = await self.get_top_ranked_prefixes(prefix, 0, max_completions - 1)
            if len(res) > max_completions:
                await self.delete_least_ranked_prefix(prefix)
                prefix_added = True

        if prefix_added:
            return Prefix_Change.Added

        return Prefix_Change.Modified


    async def get_value_for_completion(self, name: string, completion:string ):
        return await self.cache.zscore(name, completion)

async def fill_cache(processor: PrefixProcessor, prefix_key: string, prefix_length_key: string, prefix_length_value: int, completions_key: string ) -> MemoryCache:
    cache = MemoryCache()
    prefix_key = 'prefix'
    prefix_array = 'completions'
    variant_key = 'variant'
    variant_counter = 'counter'
    async for prefix_doc in processor.get_limited_prefixes(prefix_key, prefix_length_key,  prefix_length_value, completions_key):
        mapping = {}
        for pair in prefix_doc[prefix_array]:
            mapping[pair[variant_key]] = pair[variant_counter]

        await cache.add_data(prefix_doc[prefix_key], mapping)

    return cache
