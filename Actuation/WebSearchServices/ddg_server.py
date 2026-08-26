import uvicorn
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from duckduckgo_search import DDGS

app = FastAPI(title="DuckDuckGo Search Sidecar")
ddgs = DDGS()

class SearchRequest(BaseModel):
    query: str
    max_results: int = 5
@app.get("/health")
def health():
    return {"status": "ready"}

@app.post("/search")
def search(req: SearchRequest):
    try:
        results = list(ddgs.text(req.query, max_results=req.max_results))
        return {"results": results}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

if __name__ == "__main__":
    uvicorn.run(app, host="127.0.0.1", port=8000, log_level="warning")