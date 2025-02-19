# import asyncio

#https://stackoverflow.com/questions/33824359/read-file-line-by-line-with-asyncio
import string


# from collections import OrderedDict

from core.engine import Forest
#https://python-school.ru/blog/data-structures/sort-dict/
# https://vibhu4agarwal.hashnode.dev/python-cs-stl-data-structures-and-their-
#https://www.freecodecamp.org/news/lambda-sort-list-in-python/
# https://testdriven.io/blog/fastapi-mongo/
# https://motor.readthedocs.io/en/stable/tutorial-asyncio.html
# https://pymongo.readthedocs.io/en/stable/tutorial.html
from core.engine_utils import (normalize_string, create_db_documents_generator,
                               get_num_prefixes_in_completion_forest)

from core.dbengine import DocumentRepository
from core.exception_types import CollectionDeletionError


def delete_existent_collection(db_name:string, coll_name:string, repo: DocumentRepository) -> None:
    coll_names = rep.get_collection_names(db_name)

    if coll_name in coll_names:
        if not repo.drop_collection(db_name, coll_name):
            raise CollectionDeletionError("Unable to delete collection {} from database {}".format(db_name, db_name))


def output_progress(num_documents: int, coll_name:string) -> None:
    print("{} documents has written to collection {}".format(num_documents, coll_name))

if __name__ == "__main__":
    try:
        # lst111 = [1, 2, 3, 4, 5, 6, 7]
        # sliced = lst111[0: 110]

        # file_reader = create_file_reader('data/english_words.txt')
        # for word in file_reader:
        #     print(word)


        frst = Forest()
        frst.insert_string(normalize_string("      cat         "))
        frst.insert_string(normalize_string("camel"))
        frst.insert_string(normalize_string("camera"))
        frst.insert_string(normalize_string("carnival"))
        frst.insert_string(normalize_string("car"))

        frst.insert_string(normalize_string("abbreviate"))
        frst.insert_string(normalize_string("adventure"))
        frst.insert_string(normalize_string("accumulator"))

        # print('num words:  '+ str(get_num_words_in_completion_forest(frst)))
        # profiling https: // habr.com / ru / companies / vk / articles / 202832 /


        # res = get_documents_number("temp_mono", "eng")
        # print('num documents:' + str(res))
        # res = drop_collection("temp_mono", "eng")
        # print('delete collection:' + str(res))

        # tree = frst.get_prefix_tree('c')
        # if tree:
        #     for tree_key in tree.get_keys():
        #         subtree = frst.get_prefix_tree(tree_key)
        #         compMap = subtree.get_completion_map()
        #         for key_map, val_map in compMap.items():
        #             print(json.dumps({'prefix': key_map, 'query': val_map}))

                # print(json.dumps(subtree.__dict__))

        # client = MongoClient("localhost", 27017)
        # db = client["temp_mono"]
        # collection = db["eng"]

        # res = proc.modify_variant_counter('completions', 'variant', 'camel',
        #                             'counter', 10)
        # print(res)


        # match =  create_match_aggregate("prefix", "ca")
        # unwind = create_unwind_aggregate("completions")
        # srt = create_sort_aggregate("completions", "counter", Sort_Order.Decrease)
        # print(srt)
        # lmt = create_limit_aggregate(3)
        # grp = create_group_aggregate("completions")
        # # pipeline = create_pipeline(match, unwind, srt, lmt, grp)
        # # print(pipline)
        # res =rep.perform_pipeline_operation("temp_mono", "eng",
        #                                     create_pipeline(match, unwind, srt, lmt, grp), 'completions')
        # print(res)

        # filter_object = format_filter_object("prefix", format_regex_prefix("c"))
        # update_object = create_object_to_add('completions', 'variant',  'can', 'counter', 10)

        # res = rep.find_all_documents("temp_mono", "eng", filter_object, 'completions')
        # print(type(res))

        # res = rep.modify_all_documents("temp_mono", "eng", filter_object, update_object)
        # print("modify_all_documents " + str(res))

        # res = collection.find()
        #
        # for r in res:
        #     pprint.pprint(r)
        #
        # gen = create_db_documents_generator(frst)

        # frst = create_prefix_forest_from_file('data/english_words.txt') UNCOMMENT
        # num_words = get_num_words_in_completion_forest(frst)
        # print('Words number:{}'.format(num_words))
        num_words = get_num_prefixes_in_completion_forest(frst)
        print('Words number:{}'.format(num_words))

        rep = DocumentRepository()
        # proc = PrefixProcessor(rep, "temp_mono", "eng_prefix")
        delete_existent_collection("temp_mono", "eng_prefix", rep)

        gen = create_db_documents_generator(frst)

        num_insertions = rep.insert_documents_in_database(gen, output_progress,"temp_mono", "eng_prefix", 5)
        # i = 1
        # for dmp in gen:
        #     # print(dmp)
        #     res = collection.insert_one(dmp)
        #     print(res.acknowledged)
        #     print(res)
        #     i += 1
        #     if i >= 3:
        #         break

        num_docs = rep.get_documents_number("temp_mono", "eng_prefix")
        if num_docs != num_insertions:
            print("Number of words {} in file doesn't match to number of documents in the collection {}".format(num_words, num_docs))
        else:
            print("Creation of collection completed")

        # collection.aggregate()
        # update_res = collection.update_many({"query.query": "car"}, {"$set": {"query.$.counter": 25}})
        # update_res.modified_count update_res.matched_count

        # pprint.pprint(collection.find_one())
        # results = collection.find_one({ 'prefix': 'ca'})
        # results = collection.find()

        # frst.increment_suggestion_counter("camera")
        # suglist = frst.get_suggestions("ca")

        # https: // tproger.ru / articles / raspredelenie - pamjati - v - python - skolko - i - v - kakih - sluchajah - zanimajut - tipy - dannyh
        # сколько памяти занимает программа на python

        # print('size of forest:', sys.getsizeof(suglist))
        #
        # str1 = 'Hello'
        # print('size of str1:', sys.getsizeof(str1))
        # str1 = 'Hello again'
        # print('size of str1:', sys.getsizeof(str1))
        # str111 = [1, 2, 3, 4, 5]
        # print('len of str111:', len(str111))

        # QueryInfo = namedtuple('QueryInfo', 'query counter')
        # lst: list[QueryInfo] = []
        #
        # lst.append(QueryInfo(query='First', counter=10))
        # lst.append(QueryInfo(query='Second', counter=20))
        # lst.append(QueryInfo(query='Third', counter=30))
        # lst.append(QueryInfo(query='Fourth', counter=40))
        #
        # generator(limit: int)-> int:
        # try:
        #     lst.remove(QueryInfo(query='First', counter=10))
        # except Exception as ex:
        #     print(ex.args)

        # lst.sort(key = lambda x: x.counter, reverse=True)



    except Exception as ex:
        print(ex.args)


