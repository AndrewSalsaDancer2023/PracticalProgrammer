import string
from  core.dbengine_async import AsyncDocumentRepository
# from core.abstractrepository import AbstractDocumentRepository
from core.engine_utils import (create_search_word_filter, create_modify_word_count_key,
                               create_modify_word_count_filter, create_modify_word_count_predicate,
                               create_increment_counter_predicate,
                               create_project_for_prefix_aggregate, create_match_for_prefix_aggregate,
                               format_filter_object, format_regex_prefix, create_object_to_add,
                               create_match_aggregate, create_unwind_aggregate, create_sort_aggregate,
                               Sort_Order,create_limit_aggregate, create_group_aggregate,
                               create_project_aggregate, create_pipeline)

class PrefixProcessor:
    def __init__(self, repository: AsyncDocumentRepository, db_name: string, coll_name: string):
        self.repository = repository
        self.db_name = db_name
        self.coll_name = coll_name

    def get_document_repository(self) -> AsyncDocumentRepository:
        return self.repository

    async def modify_variant_counter(self, array_key: string, object_key: string, object_value: string,
                            object_count_key:string, object_count_value: int) -> bool:

        srch_filter = create_search_word_filter(array_key, object_key)
        srch_filter = create_modify_word_count_filter(srch_filter, object_value)

        cnt_filter = create_modify_word_count_key(array_key, object_count_key)
        cnt_predicate = create_modify_word_count_predicate(cnt_filter, object_count_value)

        return await self.repository.modify_all_documents(self.db_name, self.coll_name, srch_filter, cnt_predicate)


    async def add_variant(self, prefix_key: string, prefix_value: string, array_key: string, variant_key: string,
                           variant_value: string, object_count_key:string, object_count_value: int) -> bool:

        filter_object = format_filter_object(prefix_key, format_regex_prefix(prefix_value))
        update_object = create_object_to_add(array_key, variant_key, variant_value, object_count_key, object_count_value)
        return await self.repository.modify_all_documents(self.db_name, self.coll_name, filter_object, update_object)

    async def get_completions(self, prefix: string, result_key: string) ->list:
        match =  create_match_aggregate("prefix", prefix)
        unwind = create_unwind_aggregate("completions")
        srt = create_sort_aggregate("completions", "counter", Sort_Order.Decrease)
        # print(srt)
        lmt = create_limit_aggregate(5)
        grp = create_group_aggregate("completions")
        proj = create_project_aggregate("completions", "variant")
        pipeline = create_pipeline(match, unwind, srt, lmt, grp, proj)
        # print(pipline)
        res = await self.repository.perform_pipeline_operation(self.db_name, self.coll_name, pipeline, result_key)
        return res

    async def get_limited_prefixes(self, prefix_key: string, prefix_length_key:string, prefix_length_value: int, completions_key: string ):
        proj = create_project_for_prefix_aggregate(prefix_key, prefix_length_key, completions_key)

        match =  create_match_for_prefix_aggregate(prefix_length_key, prefix_length_value)

        pipeline = create_pipeline(proj, match)
        # print(pipline)
        async for res in self.repository.perform_pipeline_operation_for_fetch(self.db_name, self.coll_name, pipeline):
            yield res

    # https://www.geeksforgeeks.org/mongodb-increment-operator-inc/