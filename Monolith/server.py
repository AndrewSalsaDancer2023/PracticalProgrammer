from fastapi import FastAPI, Depends
from fastapi.responses import JSONResponse
from contextlib import asynccontextmanager

from pyinstrument import Profiler
from starlette.middleware.base import BaseHTTPMiddleware, RequestResponseEndpoint
from starlette.requests import Request
from starlette.responses import Response

from aiologger.handlers.files import AsyncFileHandler
from tempfile import NamedTemporaryFile
from core.dbengine_async import AsyncDocumentRepository
from core.prefix_processor import PrefixProcessor
from core.engine_utils import normalize_string #, convert_bytes_list_to_response
from core.memory_cache import fill_cache, MemoryCache

from pydantic import BaseModel
# import asyncio
from aiologger import  Logger
import time
# import core.dbengine_async
# configure_logging(level=logging.INFO, log_name='bugs_log')
# logger = logging.getLogger(__name__)

db_name = "temp_mono"
coll_name = "eng"
# coll_name = "eng_prefix"
completions_max_variants = 5

doc_repository = AsyncDocumentRepository()
processor = PrefixProcessor(doc_repository, db_name, coll_name)
print(__name__)

logger = Logger.with_default_handlers(name='my-logger')
temp_file = NamedTemporaryFile(prefix = 'log_', suffix = '_file', dir = 'logs', delete=False)
handler = AsyncFileHandler(filename=temp_file.name)
logger.add_handler(handler)
cache: MemoryCache
# profiling https://habr.com/ru/companies/vk/articles/202832/

@asynccontextmanager
async def lifespan(application: FastAPI):
    try:
        # read_task = asyncio.create_task(read_json(config_path))
        # config_content = await read_task
        global cache
        await logger.info('Application start')
        async for it in processor.get_limited_prefixes('prefix', 'prefixLength', 5, 'completions'):
            print(it)
        cache = await fill_cache(processor, 'prefix', 'prefixLength', 5, 'completions')

        await cache.update_completion_counter('cat', 5)

        res = await cache.get_value_for_completion('c', 'cat')

        # res = await cache.delete_least_ranked_prefix('c')
        # val = int(res[0][1])
        print(type(res))
        # if len(res) > 0:
        #     print(res[0][0].decode('UTF-8')) #name
        #     print(int(res[0][1])) # value

        # game_timer = GameTimer(dog_game.get_tick_period())
        # game_timer.start(dog_game.timer_handler)
        yield
        # await repository.dispose_repository()

        await logger.info('Application shutdown')
        await logger.shutdown()

    except Exception as ex:
        await logger.exception(ex.args)

# https://github.com/KelvinSajere/blog-posts/blob/main/profiling-fastapi/profile.html
# https://stackademic.com/blog/profiling-fastapi-for-ultimate-performance-tuning
class PyInstrumentMiddleWare(BaseHTTPMiddleware):
    async def dispatch(self, request: Request, call_next: RequestResponseEndpoint) -> Response:
        profiler = Profiler(interval=0.001, async_mode="enabled")
        profiler.start()
        start = time.time()
        response = await call_next(request)
        end = time.time()
        profiler.stop()
        profiler.write_html("profile.html")
        await logger.info("Time: %.03f s" % (end - start))
        return response

app = FastAPI(lifespan=lifespan)
app.add_middleware(PyInstrumentMiddleWare)
result_key = 'completions'

def create_success_response(content: dict):
    return JSONResponse(status_code=200, content=content)

# def create_success_records_response(content: list):
#     return JSONResponse(status_code=200, content=content)

@app.get("/api/v1/completions/{prefix}")
async def get_completions(prefix: str):
    try:
        prefix = normalize_string(prefix)
        if not prefix:
            return create_success_response({"error": "Empty request"})

        res = await cache.get_top_ranked_prefixes(prefix, 0, completions_max_variants - 1)
        # if res:
        #     res = convert_bytes_list_to_response(res)
        if not res:
            res = await processor.get_completions(prefix, result_key)

        return create_success_response({result_key: res})
    except Exception as ex:
        await logger.info(ex.args)


@app.post("/api/v1/completions/{prefix}")
async def add_completions(prefix: str):
    prefix = normalize_string(prefix)
    return create_success_response({result_key: prefix, "added" : "success"})

# class Item(BaseModel):
#     prefix: str

@app.put("/api/v1/completions/{prefix}")
async def update_item(prefix: str):
    prefix = normalize_string(prefix)
    return create_success_response({result_key: prefix, "updated" : prefix})


if __name__ == "__main__":
    import uvicorn

    uvicorn.run(app, host="0.0.0.0", port=8080)
    #uvicorn.run(app, host="localhost", port=8080)