#include "JsonSenderLinux.hpp"
#include "string.h"

using namespace std;

int main(){
    JsonSender sender;
    string userpromt = "Give the steps to make a pineapple cake";
    string sysData = "\
        You are a task execution planner. When given a task, break it down into sequential steps.\
        Return the response as a JSON object with the following structure:\
        {\
        “task_id”: “<unique identifier for the task>”,\
        “task_name”: “<name of the task>”,\
        “steps”: {\
        “<step_id>”: {\
        “id”: “<step_id>”,\
        “name”: “<short name of the step>”,\
        “tool”: “<tool name if needed, otherwise null>”,\
        “description”: “<detailed description of what happens in this step>”\
        }\
        },\
        “step_descriptions”: {\
        “<step_id>”: “<same description as in steps[step_id].description>”\
        }\
        }\
        Rules:\
            The key of each entry in “steps” must equal the “id” field inside it.\
            The key of each entry in “step_descriptions” must match the corresponding step’s id.\
            Each step must be in its own separate dictionary inside “steps”.\
            step_ids are sequential integers as strings: “1”, “2”, “3”, …\
            The “tool” field is null if no external tool is needed for that step.\
            Descriptions in “step_descriptions” must be identical to the ones in “steps”.";


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
}   