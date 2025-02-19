import pymongo
import string

from core.abstractrepository import AbstractDocumentRepository, Pipeline


# https://stackoverflow.com/questions/13449874/how-to-sort-array-inside-collection-record-in-mongodb
class DocumentRepository(AbstractDocumentRepository):
    def __init__(self, host: string = "mongodb://localhost:27017/"):
        self.host = host
        self.client = pymongo.MongoClient(host)

    # https://www.tutorialspoint.com/get-index-of-given-element-in-array-field-in-mongodb
    def drop_collection(self, db_name: string, coll_name: string) -> bool:
        db = self.client[db_name]
        # collection = db[coll_name]
        res = db.drop_collection(coll_name)
        return res['ok'] != 0.0

    def get_documents_number(self, db_name: string, coll_name: string) -> int:
        db = self.client[db_name]
        collection = db[coll_name]
        return collection.count_documents({})

    def get_collection_names(self, db_name: string) -> list[string]:
        db = self.client[db_name]
        return db.list_collection_names()

    # https://pymongo.readthedocs.io/en/stable/examples/aggregation.html
    # https: // pymongo.readthedocs.io / en / stable / examples / bulk.html
    # https: // www.google.com / search?q = pymongo + bulk + write & oq = pymongo + bulk + w & gs_lcrp = EgZjaHJvbWUqBwgBEAAYgAQyBggAEEUYOTIHCAEQABiABDIICAIQABgWGB4yCAgDEAAYFhgeMggIBBAAGBYYHjIICAUQABgWGB4yCAgGEAAYFhgeMggIBxAAGBYYHjIICAgQABgWGB4yCggJEAAYChgWGB7SAQkxMzU4NGowajeoAgiwAgE & sourceid = chrome & ie = UTF - 8
    def insert_documents_in_database(self, gen, callback, db_name: string, coll_name: string, doc_limit: int = 100) -> int:
        db = self.client[db_name]
        collection = db[coll_name]
        cur_content = []
        num_doc_writes = 0
        for dmp in gen:
            cur_content.append(dmp)
            if len(cur_content) >= doc_limit:
                res = collection.insert_many(cur_content)
                num_doc_writes += len(res.inserted_ids)
                callback(num_doc_writes, coll_name)
                if len(res.inserted_ids) != len(cur_content):
                    return num_doc_writes
                cur_content.clear()

        if len(cur_content) > 0:
            res = collection.insert_many(cur_content)
            num_doc_writes += len(res.inserted_ids)
            callback(num_doc_writes, coll_name)
        return num_doc_writes

# pipeline = [
#     {"$unwind": "$tags"},
#     {"$group": {"_id": "$tags", "count": {"$sum": 1}}},
#     {"$sort": SON([("count", -1), ("_id", -1)])},
# ]
    def perform_pipeline_operation(self, db_name: string, coll_name: string, pipeline: Pipeline, result_key: string) -> list[dict]:
        db = self.client[db_name]
        collection = db[coll_name]

        cursor = collection.aggregate(pipeline)
        for doc in cursor:
            return doc[result_key]


    def find_document(self, db_name: string, coll_name: string, search_filter: dict, compl_key: string) -> list:
        db = self.client[db_name]
        collection = db[coll_name]
        result = collection.find_one(search_filter)
        if not result:
            return []
        return result[compl_key]

    def find_all_documents(self, db_name: string, coll_name: string, search_filter: dict, compl_key: string) -> list[list]:
        db = self.client[db_name]
        collection = db[coll_name]
        cursor = collection.find(search_filter)

        res = []
        for doc in cursor:
            res.append(doc[compl_key])

        return res

    def modify_all_documents(self, db_name: string, coll_name: string, search_filter: dict, update_filter: dict) -> bool:
        db = self.client[db_name]
        collection = db[coll_name]
        res = collection.update_many(search_filter, update_filter)

        return res.modified_count > 0


    # def modify_word_count(self, db_name: string, coll_name: string, search_filter: dict, update_filter: dict) -> bool:


# {
#     prefix: "c",
#     keys: ["c", "ca", "car", "cat"],
#     c : [{"car" : 1} , {"cat" : 10}],
#     ca : [ {"car" : 1}, { "cat" : 1}],
#     car : [{"car" : 1}],
#     cat : [{"cat":1}]
# }