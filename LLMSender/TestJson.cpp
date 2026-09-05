#include "JsonSenderLinux.hpp"
#include "LLMReciever.hpp"
#include "string.h"

using namespace std;

int main(){
    JsonSender sender;
    LLMReciever reciever;


    string userpromt = "open the web browser and find the birth day of Ronaldo in my system";
    std::string sysData = R"(You are a task execution planner. When given a task, break it down into sequential steps.
    Return ONLY a valid raw JSON object (no markunique identifier for no explanation) with EXACTLY this structure:
    {
    "task_id": "<unique identifier for the task>",
    "task_name": "<name of the task>",
    "steps": {
        "<step_id>": {
        "id": "<step_id>",
        "name": "<short name of the step>",
        "tool": "<tool name if needed, otherwise null>",
        "arguments": "<JSON object of tool parameters if tool is set, otherwise null>",
        "description": "<detailed description of what happens in this step>"
        }
    },
    "step_descriptions": {
        "<step_id>": "<same description as in steps[step_id].description>"
    }
    }

    Rules:
    - The key of each entry in "steps" must equal the "id" field inside it.
    - The key of each entry in "step_descriptions" must match the corresponding step's id.
    - step_ids are sequential integers as strings: "1", "2", "3", ...
    - If a step needs no external tool, set both "tool" and "arguments" to null.
    - If "tool" is set, "arguments" MUST be a JSON object whose keys exactly match the parameter names of that tool, include ALL required parameters of that tool, and use the exact declared types (string, integer, boolean, or array of strings). Never invent or omit parameters.
    - The tool's parameters are defined in the "tools" array appended below; match names and types against it.
    - Descriptions in "step_descriptions" must be identical to the ones in "steps".
    )";



    string apiKey = "sk-OzgzQqIc8azSnEH9Lzn5EYx1mLabqH2tizw99nVWGdTD0KE3";
    string endpoint = "https://api.gapgpt.app/v1/chat/completions";

    json tools = sender.BuildToolsSchema();
    string result = sender.SendDataToLLM(
        apiKey,
        endpoint,
        userpromt,
        sysData,
        tools,
        "",
        "",
        "gpt-4o"
    );
    cout << result << endl;

    std::vector<ActionItem> actionItems;
    std::vector<Description> descriptions;
    reciever.parseLLMResponse(result, actionItems, descriptions);
}   