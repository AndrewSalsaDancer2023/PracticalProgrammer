# from collections import namedtuple
import string
from memory_profiler import profile
from typing import Optional
from enum import Enum

#QueryInfo = namedtuple('QueryInfo', 'query counter')

class QueryInfo(dict):
    def __init__(self, variant : string, counter: int = 0):
        self.variant = variant
        self.counter = counter
        dict.__init__(self, variant=variant, counter=counter)

    def increment_by(self, value: int = 1) -> None:
        self.counter += value

AutocompletionMap = dict[string, list[QueryInfo]]
Suggestions = list[string]

def get_variant(info: QueryInfo) -> string:
    return info.variant

class PrefixHashTree:
    def __init__(self):
        self.completionMap: AutocompletionMap = {}

    def num_keys(self):
        return len(self.completionMap.keys())

    def get_keys(self):
        return self.completionMap.keys()

    def get_completion_map(self) ->AutocompletionMap:
        return self.completionMap

    def insert_string(self, word: string, length_limit: int = 30) -> None:
        query = QueryInfo(word, 0)
        for i in range(min(len(word),length_limit)):
            prefix = word[ : i+1]
            val = self.completionMap.get(prefix)
            if val is None:
                self.completionMap[prefix] = [query]
            else:
                self.completionMap[prefix].append(query)

    def get_num_completions(self, word: string) -> int:
        completions = self.completionMap.get(word)
        if not completions:
            return 0

        return len(completions)

    def get_suggestions(self, word: string, words_limit: int = 5) -> Suggestions:
        completions = self.completionMap.get(word)
        if not completions:
            return []

        completions.sort(key=lambda x: x.counter, reverse=True)
        suggestions = completions[ : words_limit]
        return list(map(get_variant, suggestions))

    def increment_suggestion_counter(self, word: string, value: int = 1, length_limit: int = 30) -> None:
        word = word.strip()
        if not word:
            return

        prefix = word[0]
        completions = self.completionMap.get(prefix)
        if completions is None:
            return

        res = filter(lambda qinfo: qinfo.variant == word, completions)
        suggestions = list(res)
        if not len(suggestions):
            return

        suggestions[0].increment_by(value)


AutocompletionForest = dict[string, PrefixHashTree]

class Forest:
    def __init__(self):
        self.forest: AutocompletionForest = {}

    def num_keys(self):
        return len(self.forest.keys())

    def get_keys(self):
        return self.forest.keys()

    def get_prefix_tree(self, word: string) -> Optional[PrefixHashTree]:
        return self.forest.get(word[0])

    def insert_string(self, word: string, length_limit: int = 30) -> None:
        word = word.strip()
        if not word:
            return

        val = self.forest.get(word[0])
        if val is None:
            self.forest[word[0]] = PrefixHashTree()

        self.forest[word[0]].insert_string(word, length_limit)

    def get_suggestions(self, word: string, words_limit: int = 5) -> Suggestions:
        word = word.strip()
        if not word:
            return []

        tree = self.forest.get(word[0])
        if not tree:
            return []

        return tree.get_suggestions(word, words_limit)

    def increment_suggestion_counter(self, word: string, value: int = 1, length_limit: int = 30) -> None:
        word = word.strip()
        if not word:
            return

        tree = self.forest.get(word[0])
        if not tree:
            return

        tree.increment_suggestion_counter(word, value, length_limit)

# for k, v in tel.items():
#     print(k, v)
