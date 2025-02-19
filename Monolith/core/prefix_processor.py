import string
from core.abstractrepository import AbstractDocumentRepository
from core.engine_utils import (create_search_word_filter, create_modify_word_count_key,
                               create_modify_word_count_filter, create_modify_word_count_predicate,
                               format_filter_object, format_regex_prefix, create_object_to_add)

class PrefixProcessor:
    def __init__(self, repository: AbstractDocumentRepository, db_name: string, coll_name: string):
        self.repository = repository
        self.db_name = db_name
        self.coll_name = coll_name

    def get_document_repository(self) -> AbstractDocumentRepository:
        return self.repository

    def modify_variant_counter(self, array_key: string, object_key: string, object_value: string,
                            object_count_key:string, object_count_value: int) -> bool:

        srch_filter = create_search_word_filter(array_key, object_key)
        srch_filter = create_modify_word_count_filter(srch_filter, object_value)

        cnt_filter = create_modify_word_count_key(array_key, object_count_key)
        cnt_predicate = create_modify_word_count_predicate(cnt_filter, object_count_value)

        return self.repository.modify_all_documents(self.db_name, self.coll_name, srch_filter, cnt_predicate)


    def add_variant(self, prefix_key: string, prefix_value: string, array_key: string, variant_key: string,
                           variant_value: string, object_count_key:string, object_count_value: int) -> bool:

        filter_object = format_filter_object(prefix_key, format_regex_prefix(prefix_value))
        update_object = create_object_to_add(array_key, variant_key, variant_value, object_count_key, object_count_value)
        return self.repository.modify_all_documents("temp_mono", "eng", filter_object, update_object)