from abc import ABC, abstractmethod
import string

Pipeline = list[dict]

class AbstractDocumentRepository(ABC):
    @abstractmethod
    def drop_collection(self, db_name: string, coll_name: string) -> bool:
        pass
    @abstractmethod
    def get_documents_number(self, db_name: string, coll_name: string) -> int:
        pass
    @abstractmethod
    def get_collection_names(self, db_name: string) -> list[string]:
        pass
    @abstractmethod
    def insert_documents_in_database(self, gen, callback, db_name: string, coll_name: string, doc_limit: int = 100) -> int:
        pass
    @abstractmethod
    def perform_pipeline_operation(self, db_name: string, coll_name: string, pipeline: Pipeline, result_key: string) -> list[dict]:
        pass
    @abstractmethod
    def find_document(self, db_name: string, coll_name: string, search_filter: dict, compl_key: string) -> list:
        pass
    @abstractmethod
    def find_all_documents(self, db_name: string, coll_name: string, search_filter: dict, compl_key: string) -> list[list]:
        pass
    @abstractmethod
    def modify_all_documents(self, db_name: string, coll_name: string, search_filter: dict, update_filter: dict) -> bool:
        pass
