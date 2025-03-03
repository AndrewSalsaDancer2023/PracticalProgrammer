import string
from core.engine import Forest, PrefixHashTree
from core.abstractrepository import Pipeline

from enum import IntEnum
class Sort_Order(IntEnum):
    Decrease = -1
    Increase = 1

def normalize_string(entire_string: string) -> string:
    chars_to_del = ' \r\n\t'
    return entire_string.strip(chars_to_del)

def convert_bytes_list_to_response(bytes_list: list[bytes]) -> list[dict]:
    res = []
    for el in bytes_list:
        res.append({'variant': el.decode('UTF-8')})
    return res

def create_db_documents_generator(entire_forest: Forest) -> dict:
    for key in entire_forest.get_keys():
        tree = entire_forest.get_prefix_tree(key)
        if not tree:
            continue

        comp_map = tree.get_completion_map()
        for key_map, val_map in comp_map.items():
            yield {'prefix': key_map, 'completions': val_map}
            # yield json.dumps({'prefix': key_map, 'query': val_map})

def create_file_reader(path: string) -> string:
    with open(path) as file:
        for line in file:
            yield normalize_string(line)

def create_prefix_forest_from_file(path: string) -> Forest:
    file_reader = create_file_reader(path)
    forest = Forest()
    for word in file_reader:
        forest.insert_string(normalize_string(word))

    return forest

# https://www.geeksforgeeks.org/python-get-the-first-key-in-dictionary/
def get_num_words_in_completion_tree(tree: PrefixHashTree) -> int:
    first_key = next(iter(tree.get_keys()))
    if not first_key:
        return 0

    return tree.get_num_completions(first_key[0])

# @profile
def get_num_words_in_completion_forest(forest: Forest) -> int:
    num_words = 0
    for key in forest.get_keys():
        tree = forest.get_prefix_tree(key)
        if not tree:
            continue

        num_words += get_num_words_in_completion_tree(tree)

    return num_words

def get_num_prefixes_in_completion_forest(forest: Forest) -> int:
    num_words = 0
    for key in forest.get_keys():
        tree = forest.get_prefix_tree(key)
        if not tree:
            continue

        num_words += len(tree.completionMap)

    return num_words


def format_regex_prefix(prefix: string) -> string:
    return "^{}".format(prefix)

# regex_predicate example ^c
def format_filter_object(key: string, regex_predicate: string) -> dict[string, string]:
    return { key: { "$regex": regex_predicate} }

#key - completions, variant_key - variant, variant_value - 'cat', value_key - counter, value - 10
def create_object_to_add(key: string, variant_key: string, variant_value:string, value_key: string, value: int) -> dict[string, string]:
    return { "$addToSet": { key: { variant_key: variant_value, value_key: value} }}

def create_modify_word_count_filter(word_key: string, word_value: string) -> dict[string, string]:
    return { word_key: word_value }

def create_modify_word_count_key(part1_key: string, part2_key: string) -> string:
    return part1_key+'.$.'+part2_key

def create_search_word_filter(part1_key: string, part2_key: string) -> string:
    return part1_key+'.'+part2_key

def create_modify_word_count_predicate(counter_key: string, new_counter_value: int) -> dict[string, string]:
    return { "$set" : { counter_key: new_counter_value  } }

def create_increment_counter_predicate(counter_key: string, increment_value: int) -> dict[string, string]:
    return {"$inc": {counter_key: increment_value}}

def create_match_aggregate(prefix_key: string, prefix_value: string) -> dict:
    return {"$match": {prefix_key: prefix_value}}

def create_unwind_aggregate(array_key: string) -> dict:
    return { "$unwind": "${}".format(array_key) }

def create_sort_aggregate(array_key: string, counter_key: string, sort_order: Sort_Order = Sort_Order.Decrease) -> dict:
    return {"$sort": {'{}.{}'.format(array_key, counter_key) : int(sort_order) }}

def create_limit_aggregate(limit_value: int) -> dict:
    return {"$limit": limit_value}

def create_group_aggregate(array_key: string) -> dict:
    return {"$group": {"_id": "$_id", array_key : {"$push": "${}".format(array_key)} }}

def create_project_aggregate(array_key: string, counter_key: string) -> dict:
    return {"$project": {'{}.{}'.format(array_key, counter_key) : 1, "_id" : 0 }}

def create_pipeline(*args) -> Pipeline:
    res = []
    for arg in args:
        res.append(arg)

    return res

def create_project_for_prefix_aggregate(prefix_key: string, prefix_length_key: string, completions_key: string) -> dict:
    return {"$project": {"_id": 0, prefix_key: 1, prefix_length_key: {"$strLenCP": "${}".format(prefix_key)}, completions_key: 1}}

def create_match_for_prefix_aggregate(prefix_length_key: string, prefix_length_value: int) -> dict:
    return {"$match": { prefix_length_key: {"$lte" : prefix_length_value} }}

# https://www.geeksforgeeks.org/how-to-get-string-length-in-mongodb/