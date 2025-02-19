# import asyncio

#https://stackoverflow.com/questions/33824359/read-file-line-by-line-with-asyncio
import string

#https://python-school.ru/blog/data-structures/sort-dict/
# https://vibhu4agarwal.hashnode.dev/python-cs-stl-data-structures-and-their-
#https://www.freecodecamp.org/news/lambda-sort-list-in-python/
# https://testdriven.io/blog/fastapi-mongo/
# https://motor.readthedocs.io/en/stable/tutorial-asyncio.html
# https://pymongo.readthedocs.io/en/stable/tutorial.html
from core.engine_utils import (create_db_documents_generator,
                               get_num_prefixes_in_completion_forest, create_prefix_forest_from_file)

from core.dbengine import DocumentRepository
from core.exception_types import CollectionDeletionError


def delete_existent_collection(database_name:string, collection_name:string, repo: DocumentRepository) -> None:
    coll_names = repo.get_collection_names(db_name)

    if coll_name in coll_names:
        if not repo.drop_collection(database_name, collection_name):
            raise CollectionDeletionError("Unable to delete collection {} from database {}".format(db_name, db_name))


def output_progress(num_documents: int, collection_name:string) -> None:
    print("{} documents has written to collection {}".format(num_documents, collection_name))

def create_prefix_database(database_name: string, collection_name: string, words_file_path:string):
    frst = create_prefix_forest_from_file(words_file_path)
    num_words = get_num_prefixes_in_completion_forest(frst)
    print('Words number:{}'.format(num_words))
    rep = DocumentRepository()
    delete_existent_collection(database_name, collection_name, rep)

    gen = create_db_documents_generator(frst)

    num_insertions = rep.insert_documents_in_database(gen, output_progress, database_name, collection_name, 100)

    num_docs = rep.get_documents_number(database_name, collection_name)
    if num_docs != num_insertions:
        print("Number of words {} in file doesn't match to number of documents in the collection {}".format(num_words,
                                                                                                            num_docs))
    else:
        print("Creation of collection completed")

try:
    db_name = "temp_mono"
    coll_name = "eng_prefix"
    words_dict_path = 'data/english_words.txt'
    create_prefix_database(db_name, coll_name, words_dict_path)
    # proc = PrefixProcessor(rep, "temp_mono", "eng_prefix")

except Exception as ex:
    print(ex.args)


