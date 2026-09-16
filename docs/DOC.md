# Table of Content
- [Architecture & Workflow](#architecture--workflow)
- [DataBase](#database)
    - [1. Problem Statement](#1-problem-statement)
    - [2. Storage Strategy Evaluation](#2-storage-strategy-evaluation)
        - [Alternative A: Custom Encrypted Flat Files (.json.enc)](#alternative-a-custom-encrypted-flat-files-jsonenc)
        - [Alternative B: Embedded Relational Engine (SQLite + SQLCipher)](#alternative-b-embedded-relational-engine-sqlite--sqlcipher)
        - [Storage Strategy Comparison](#storage-strategy-comparison)
    - [3. Transaction Integrity & Concurrency Architecture](#3-transaction-integrity--concurrency-architecture)
        - [ACID Guarantees](#acid-guarantees)
        - [Write-Ahead Logging (WAL) Mode](#write-ahead-logging-wal-mode)
        - [Deadlock Elimination via BEGIN IMMEDIATE](#deadlock-elimination-via-begin-immediate)
    - [4. SQLite Storage Engine: B-Trees vs. B+Trees](#4-sqlite-storage-engine-b-trees-vs-btrees)
    - [5. Performance Tuning via PRAGMA Directives](#5-performance-tuning-via-pragma-directives)
    - [6. ORM Integration Layer: Native SQLite vs. sqlite_orm](#6-orm-integration-layer-native-sqlite-vs-sqlite_orm)
    - [7. Persistence Subsystem Architecture](#7-persistence-subsystem-architecture)
        - [Core Architecture Components](#core-architecture-components)
- [Actuation Subsystem Design: MCP Action Modeling & Queue Architecture](#actuation-subsystem-design-mcp-action-modeling--queue-architecture)
    - [1. Action Object Modeling and Dispatching Service](#1-action-object-modeling-and-dispatching-service)
        - [Problem Statement](#problem-statement)
        - [Architectural Alternatives](#architectural-alternatives)
        - [Architectural Comparison](#architectural-comparison)
        - [Subsystem Handlers Layout](#subsystem-handlers-layout)
    - [2. Dynamic Action Queue & Replanning Architecture](#2-dynamic-action-queue--replanning-architecture)
        - [Problem Statement](#problem-statement-1)
        - [Evaluated Queue Topologies](#evaluated-queue-topologies)
        - [Queue Topology Trade-offs](#queue-topology-trade-offs)
    - [3. State Drift Remediation: Dual-Stage Validation Engine](#3-state-drift-remediation-dual-stage-validation-engine)
        - [The State Drift Problem](#the-state-drift-problem)
        - [Execution & Verification Workflow](#execution--verification-workflow)
        - [Core Engine Safeguards](#core-engine-safeguards)
- [Orchestrator](#orchestrator)
    - [Architectural Decision Record (ADR): Result and Error Reporting Pipeline](#architectural-decision-record-adr-result-and-error-reporting-pipeline)
        - [Options Considered](#options-considered)
        - [Comparison Matrix](#comparison-matrix)
        - [Why We Chose the Event Sink Approach](#why-we-chose-the-event-sink-approach)
        - [Architectural Trade-offs](#architectural-trade-offs)
    - [Post-Implementation Evaluation & Protocol Evolution](#post-implementation-evaluation--protocol-evolution)

# Architecture & Workflow
![Structure]()
![StructureOverview]()
![SequenceDiagram]()

<p align="center">
  <img src="./assets/Structure.png" width="300" alt="Centered image">
</p>

# DataBase
## 1. Problem Statement
The LLM interface application requires a low-latency, crash-resilient persistence layer capable of lazy-loading conversation histories, enforcing zero-trust encryption at rest, and supporting thread-safe reads and writes without freezing the UI.

---
## 2. Storage Strategy Evaluation
### Alternative A: Custom Encrypted Flat Files (`.json.enc`)
* **Mechanism:** 
  * A central master registry (`index.json`) maps chat IDs to discrete encrypted files on disk (`<chat_id>.enc`).
  * On demand, the application reads the registry, fetches the specific encrypted file, runs decryption, and parses the payload into memory.
* **Pros:**
  * Zero external database dependencies; trivial to prototype.
  * Independent files allow rudimentary chat-level isolation.
* **Cons:**
  * **Zero Crash Safety:** A crash, thread termination, or power loss mid-write causes file corruption and unrecoverable data loss (atomic rename workflows via temp files add substantial I/O overhead without resolving concurrency conflicts).
  * **Cryptographic Overhead:** Requires bespoke key derivation, initialization vector (IV/nonce) management, authentication tags (HMAC / AES-GCM), and stream ciphers for every individual file transaction.
  * **High Search Latency:** Filtering, indexing, or sorting requires full sequential file scans ($O(N)$), causing severe memory and CPU overhead as conversation volume scales.
### Alternative B: Embedded Relational Engine (SQLite + SQLCipher)
* **Mechanism:**
  * An embedded, single-file relational database engine operating over structured, indexed B-Tree pages.
  * Integration with **SQLCipher** provides transparent, full-database page-level 256-bit AES encryption (protecting main storage, WAL logs, and temporary files).
* **Pros:**
  * **Crash Resilience & Atomic Commits:** Built-in write-ahead logging (WAL) guarantees transactional ACID semantics and automatic rollback upon crashes.
  * **Low-Latency Lazy Loading:** Paged B-Tree indices enable sub-millisecond range queries and message chunking ($O(\log N)$) without loading entire chat histories into memory.
  * **Zero Manual File Management:** Encapsulates indexing, state management, schema migrations, and concurrency in a unified binary file.

---
### Storage Strategy Comparison

| Metric | Flat Encrypted Files (`.json.enc`) | SQLite + SQLCipher |
| :--- | :--- | :--- |
| **Crash Safety** | **Hazardous**<br>Interrupted writes cause file truncation and corruption; lacks transactional rollback | **High (ACID Compliant)**<br>Atomic commits and WAL journal ensure automatic recovery upon failure |
| **Lazy Loading** | **Poor / Fragmented**<br>Requires custom file splitting, index mapping, and full-file parsing per chat | **Optimal ($O(\log N)$)**<br>Native SQL range queries (`LIMIT` / `OFFSET`) read only requested pages from disk |
| **Encryption** | **Complex & Fragile**<br>Manual application-level implementation of IVs, salts, MAC validation, and stream ciphers | **Transparent & Native**<br>Page-level 256-bit AES encryption handled automatically across tables, logs, and temp storage |
| **Access Latency** | **$O(N)$ Sequential Scan**<br>Requires deserializing entire JSON files into heap memory | **$O(\log N)$ Indexed Traversal**<br>Contiguous block-based I/O backed by internal B-Tree cache pages |
| **Concurrency** | **Poor**<br>Risk of file-lock conflicts, race conditions, and read-write corruption across threads | **High (WAL Mode)**<br>Concurrent multi-reader execution without blocking active background writes |

![BMTreeVsBPTree](docs/assets/BMTreeVsBPTree.png)

---
## 3. Transaction Integrity & Concurrency Architecture
### ACID Guarantees
* **Atomicity:** All operations within a transaction execute completely or roll back entirely with zero partial state mutation (all-or-nothing).
* **Consistency:** The database strictly transitions between valid states, enforcing all schema constraints, foreign key cascades, and check conditions.
* **Isolation:** Concurrent transactions execute independently without leaking dirty reads or uncommitted intermediate states.
* **Durability:** Committed transactions are flushed to persistent media via synchronized disk writes, surviving application crashes or sudden system power failure.
### Write-Ahead Logging (WAL) Mode
In default rollback journal mode, writing locks the entire database, preventing concurrent reads. Under WAL mode (`PRAGMA journal_mode = WAL;`):
* Modifications are appended sequentially to a separate log file (`<db>-wal`).
* **Non-Blocking Concurrency:** Readers read consistent point-in-time snapshots using shared memory (`<db>-shm`) without acquiring write locks, while a single writer appends updates concurrently ($N$ readers + $1$ writer).
* **Checkpointing:** Periodically, accumulated pages in the WAL file are synced and transferred back into the main `.db` file.
* **Operational Constraint:** WAL requires shared memory access primitives and is unsuitable for network-mounted filesystems (NFS/SMB).
### Deadlock Elimination via `BEGIN IMMEDIATE`
By default, SQLite uses deferred transactions (`BEGIN DEFERRED`), which acquire a read lock initially and attempt to upgrade to a write lock on the first mutating statement. If two threads read simultaneously and both attempt to write, both threads encounter unrecoverable `SQLITE_BUSY` deadlocks. 
* Executing `BEGIN IMMEDIATE` reserves the database write lock immediately.
* It serializes write-intent workflows from the first statement, ensuring no other write transaction can interleave between the initial `SELECT` and the final `UPDATE`.
---
## 4. SQLite Storage Engine: B-Trees vs. B+Trees

SQLite organizes database files into fixed-size contiguous pages (typically 4096 bytes) using two tree structures:

| Metric | Classic B-Tree (SQLite Secondary Indexes) | B+Tree (SQLite Table Storage / `rowid`) |
| :--- | :--- | :--- |
| **Payload Location** | Distributed across both internal nodes and leaf nodes | **Leaf nodes only**; internal nodes store only 64-bit routing keys |
| **Leaf Linkage** | Independent leaf pages | **Chained leaf pages** via sibling pointers for rapid sequential traversal |
| **Range Queries** | Slower; requires in-order recursive subtree traversal | **Optimized ($O(K)$)**; locates lower bound then walks linked leaf pages |
| **Lookup Latency** | $O(1)$ best-case to $O(\log N)$ worst-case | **Strictly $O(\log N)$**; consistently traverses from root to leaf |
| **SQLite Application** | Fast point lookups in secondary indexes | Main row storage organized by 64-bit signed integer `rowid` |

---
## 5. Performance Tuning via PRAGMA Directives

* **Transaction Batching:** Running operations outside explicit transactions defaults to auto-commit mode, forcing an expensive disk flush (`fsync`) per statement. Batching insertions inside an explicit transaction (`BEGIN` ... `COMMIT`) amortizes disk synchronization across thousands of records.
* **High-Performance Directives:**
	-   PRAGMA journal_mode = WAL;
		- Enables concurrent read/write operations via Write-Ahead Log
	-   PRAGMA synchronous = NORMAL;
		-  Reduces fsync frequency while maintaining WAL durability
	-   PRAGMA busy_timeout = 5000;
		- Sets sleep/retry threshold (5s) on lock contention before returning SQLITE_BUSY
	-   PRAGMA foreign_keys = ON;
		- Activates relational integrity constraints and cascading deletions
	-   PRAGMA temp_store = MEMORY;
		- Holds temporary tables, indices, and views strictly in application RAM
## 6. ORM Integration Layer: Native SQLite vs. `sqlite_orm`
`sqlite_orm` is a modern, header-only C++ library that bridges relational SQLite tables and modern C++ structures at compile time using template metaprogramming. 

| **Metric**                | **Raw SQLite C API**                                                                              | **SQLite ORM (sqlite_orm)**                                                                           |
| ------------------------- | ------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------- |
| **Type Safety**           | **None**<br>Relies on manual column index binding (`sqlite3_bind_*`, `sqlite3_column_*`)          | **Compile-Time**<br>Static type checking ensures field-to-column consistency at compile time          |
| **Resource Management**   | **Manual**<br>Requires manual statement finalization (`sqlite3_finalize`) and connection teardown | **Idiomatic RAII**<br>Automatic resource cleanup bound to C++ object lifecycles and scopes            |
| **Encapsulation**         | **Leaky**<br>Requires raw SQL strings or manual mapping functions exposing internals              | **Clean**<br>Directly supports private member accessors (getters/setters) and entity models           |
| **Throughput & Overhead** | **Maximum**<br>Zero abstraction penalty; direct access to raw C driver structs                    | **Negligible Overhead**<br>Minimal CPU cost from object hydration; compiles down to optimized queries |
| **Pitfalls**              | Parameter binding errors; SQL injection vulnerabilities from string concatenation                 | Risk of **$N+1$ query overhead** if batch joins are replaced with iterative queries                   |

## 7. Persistence Subsystem Architecture

```
[ Application / Domain Layer ]
              │
              ▼
    [ RepositoryManager ] (Facade Pattern)
      ├── [ SettingsRepository ]
      └── [ ChatRepository ]
              │
              ▼
   [ DbMappers & SecretVault ] (JSON Serialization & Transparent Field Encryption)
              │
              ▼
      [ sqlite_orm Layer ]
              │
              ▼
[ DatabaseManager (Connection & PRAGMAs) ]
              │
              ▼
   [ SQLite / SQLCipher Engine ]
```

### Core Architecture Components

![RepositoryDiagram](docs/assets/RepositoryDiagram.png)

#### 1. Separation of Concerns
- **Lifecycle & Driver Layer (`DatabaseManager`):** Encapsulates connection lifecycles, configuration pragmas, disk path resolution, and schema migrations (`sync_schema()`).
- **Data Access Layer (`SettingsRepository`, `ChatRepository`):** Encapsulates domain-specific persistence behind semantic C++ APIs (e.g., `saveHistory()`, `getMessagesForChat()`), preventing raw query logic from leaking into domain services.
- **Centralized Access Layer (`RepositoryManager`):** Implements a thread-safe Facade pattern providing unified access to all repositories across the application.
#### 2. Nested Serialization & Field-Level Security (`DbMappers`)
- **Hybrid SQL/JSON Serialization:** Custom `sqlite_orm` extensions (`statement_binder`, `row_extractor`) serialize deeply nested object trees (such as execution plans or provider options) directly into JSON text columns, avoiding expensive multi-table joins for hierarchical entities.
- **Transparent Encryption:** Sensitive attributes (such as `api_key` or credentials) are routed through `SecretVault::encrypt()` on writes and `SecretVault::decrypt()` on reads, preventing plaintext exposure in memory dumps and storage.
#### 3. Safe Resource Lifecycles & Modern Semantics
- **Move Semantics & Pointer Mapping:** Accommodates non-copyable, thread-synchronized models (such as mutex-guarded `ChatHistory`) by fetching entity IDs and loading instances via `get_pointer()` into `std::unique_ptr`, avoiding object copies while enforcing strict thread safety.
- **Modern Optional Types:** Queries for nullable single-row entities return `std::optional<T>`, eliminating raw pointers, sentinel objects, and null dereference vulnerabilities.


---

---
# Actuation Subsystem Design: MCP Action Modeling & Queue Architecture
## 1. Action Object Modeling and Dispatching Service
### Problem Statement
The Model Context Protocol (MCP) interface requires an actuation model capable of ingesting structured instructions from an LLM and dispatching them efficiently to underlying OS services (UI input injection, file system manipulation, shell commands, and observation hooks).

---
### Architectural Alternatives
#### Option A: Flat Struct with Dynamic Attribute Maps
A single concrete structure containing an action type enumeration and a dynamic key-value property bag:
* **Mechanism:** 
  * Actions contain an `ActionType` enum and an argument map (e.g., `std::unordered_map<std::string, std::any>`).
  * A central dispatcher routes actions through an extensive `switch` statement based on the enum.
* **Pros:**
  * Straightforward to implement; rapid initial prototyping.
* **Cons:**
  * **Type Erasure Overhead:** Parsing dynamic maps incurs dynamic heap allocations and runtime casting.
  * **Fragile Validation:** Parameter verification is deferred to runtime, increasing error surface.
  * **Monolithic Dispatcher:** Any MCP schema change inflates the central dispatch switch, violating maintainability.
#### Option B: Class-Based Polymorphism & Inheritance
An abstract base class defining individual interfaces for each action subtype:
* **Mechanism:**
  * A base `Action` class with derived classes (e.g., `MouseClickAction`, `OpenFileAction`).
  * **Variant 1 (External Dispatch):** Central dispatcher inspects concrete types via `dynamic_cast` or an action ID enum.
  * **Variant 2 (Command Pattern / Self-Execution):** Actions implement a virtual `execute(ExecutionContext&)` method.
* **Pros:**
  * Follows the Open-Closed Principle (OCP); new actions can be added without modifying base definitions.
  * Command pattern eliminates central dispatcher routing logic.
* **Cons:**
  * **Cache Inefficiency & Pointer Indirection:** Queue requires `std::vector<std::unique_ptr<Action>>` to prevent object slicing, introducing heap fragmentation and cache misses.
  * **Architectural Coupling:** Implementing `execute()` directly inside the action couples pure Data Transfer Objects (DTOs) to execution side effects and OS subsystem dependencies.
  * **Runtime Overhead:** Virtual method calls incur vtable indirection; downcasting via `dynamic_cast` introduces runtime latency.
#### Option C: Discriminated Unions via `std::variant` and Value Semantics
Granular, self-contained POD/DTO structs wrapped in a closed-set `std::variant`:
* **Mechanism:**
  * Each action type is defined as an independent, strongly typed struct containing only its necessary scalar parameters.
  * Actions are grouped into a discriminated union type alias:  
    `using Action = std::variant<MouseClick, KeyboardInput, RunCommand, ...>;`
  * Dispatching is handled at compile time via `std::visit` and an overloaded callable visitor pattern.
* **Pros:**
  * **Zero-Heap Value Semantics:** Eliminates `std::unique_ptr` and per-action dynamic allocations; actions sit contiguously inside `std::vector<Action>`.
  * **Compile-Time Exhaustiveness:** Adding a new type requires handling it across all visitors, caught entirely by the compiler.
  * **Branch Prediction & Inlining:** `std::visit` resolves through compiler-generated jump tables, allowing dispatch calls to inline directly into domain handlers.
* **Cons:**
  * Memory footprint per entry is bounded by the size of the largest variant alternative plus alignment padding.

---
### Architectural Comparison

| Metric | Flat Struct | Polymorphism | Discriminated Union (`std::variant`) |
| :--- | :--- | :--- | :--- |
| **Dispatch Latency** | Moderate (Branch-heavy `switch`, misprediction penalty) | Low to Moderate (Virtual table lookup indirection) | **Very Low ($O(1)$)** (Direct jump-table indexing via `std::visit`; easily inlined) |
| **Allocation Overhead** | Low (Direct scalar assignment, dynamic map heap allocations) | High (Requires `std::make_unique` per parsed action) | **Minimal (Zero-Heap)** (Constructs pure DTO values directly into contiguous vector memory) |
| **Extensibility** | Poor (Adding parameters expands payload footprint globally) | High (Add new derived classes cleanly without editing existing ones) | **High & Modular** (Add new DTO struct to the domain variant; compiler verifies exhaustive handling) |
| **Memory Footprint** | Bloated (Carries empty fields or dynamic map buckets per instance) | Fragmented (Pointers + vptr overhead per instance; scattered heap blocks) | **Compact & Cache-Friendly** ($\text{Size} = \max(\text{sizeof}(T_i)) + \text{padding}$; contiguous array layout) |
| **Type Safety** | Poor (Runtime map queries and dynamic type checking) | High (Subtype polymorphism checked at runtime) | **Complete (Compile-Time)** (Static exhaustiveness enforcement via `std::visit`) |
| **Architecture Decoupling**| Poor (Logic and dynamic validation mixed in dispatcher) | Compromised (DTOs coupled to OS logic if using Command pattern) | **Strict Separation** (Pure DTOs decoupled from subsystem execution engines) |

---
### Subsystem Handlers Layout

```

[ ActionDispatcher (std::visit) ]

├── [InputHandler] ───> Mouse Movement, Clicks, Keyboard Input, Drag & Drop

├── [FileHandler] ───> CreateFile, WriteFile, AppendFile, ReadBuffer

├── [SystemHandler] ───> Terminal Execution, PowerShell, Process Spawning

└── [ControlHandler] ───> Screen Capture, Wait/Delay, Visual Assertions, Web Queries

```

---

## 2. Dynamic Action Queue & Replanning Architecture
### Problem Statement
During autonomous agent execution, the model must inspect the environment after key operations. If visual, DOM, or system friction occurs (e.g., unexpected modal dialogs, delayed rendering, or layout shifts), the system must branch into sub-tasks or invoke remediation routines without invalidating the pending execution plan.

---
### Evaluated Queue Topologies
#### 1. Full Replanning
Discard the remaining execution queue, forward current perception data and stack traces back to the model, and request a completely refreshed action sequence.
* **Pros:** Safely resets the context during major desynchronizations or catastrophic task invalidation.
* **Cons:** Introduces prohibitive network latency and token costs for transient issues (e.g., dismissing a system prompt).
#### 2. Composite Action Pattern (Hierarchical Trees)
Actions are structured as composite nodes that recursively contain sub-actions.
* **Pros:** Standard object-oriented decomposition for deeply nested sub-tasks.
* **Cons:** 
  * Requires the LLM to emit deeply nested Abstract Syntax Trees (ASTs), significantly increasing parsing fragility and token generation overhead.
  * Recursive tree dispatching introduces call-depth overhead and complex runtime state tracking.
#### 3. In-Place Splicing (`std::vector::insert`)
Directly splice remediation sub-actions into specific offsets within an active linear `std::vector<Action>`.
* **Cons:**
  * **Memory Copies:** Inserting elements into a contiguous array incurs $O(N)$ memory shifts (`memmove`).
  * **Pointer/Iterator Invalidation:** Reallocations during expansion invalidate active iterators and index references.
  * **Telemetry Obfuscation:** Blurs baseline plans with transient hotfixes, complicating execution telemetry and deterministic replay.

#### 4. Execution Call Frame Architecture (`std::deque<ActionQueue>`)
Maintain an execution stack of action queues, where each queue acts as an isolated stack frame:
* **Mechanism:**
  * Execution consumes elements in **FIFO** order from the active queue at the top of the stack (`frames.front()`).
  * When a remediation sub-task is required, a new sub-queue is pushed as a new frame.
  * Once the top queue empties, its frame is popped, seamlessly resuming the parent queue at its paused index.
  * An underlying `std::deque` is used instead of an encapsulated `std::stack` adaptor to allow deep forward iteration for logging, stack inspection, and sanity checking without losing $O(1)$ push/pop characteristics.
---
### Queue Topology Trade-offs

| Metric                        | Full Replanning                                         | Composite Action                                               | In-Place Splicing                                          | Frame Stack (`std::deque<ActionQueue>`)                              |
| :---------------------------- | :------------------------------------------------------ | :------------------------------------------------------------- | :--------------------------------------------------------- | :------------------------------------------------------------------- |
| **Token Consumption**         | **Very High** (Regenerates entire plan on minor errors) | **High** (Requires verbose nested AST payloads)                | **Moderate** (Generates targeted patch chunks)             | **Low to Moderate** (Generates isolated, minimal remediation frames) |
| **Parsing Complexity**        | $O(N)$ (Flat array parsing)                             | $O(N)$ (Complex, recursive descent parsing)                    | $O(M)$ (Parses patch payload)                              | **$O(M)$** (Parses short, isolated remediation queues)               |
| **Dispatch Latency**          | $O(1)$ (Direct iteration)                               | $O(\text{Depth})$ (Recursive traversal across composite nodes) | $O(N)$ (Memory shift overhead on insertion)                | **$O(1)$** (Direct FIFO pop from top active frame)                   |
| **Memory Safety**             | Safe (Complete overwrite)                               | Safe (Tree-allocated)                                          | **Hazardous** (Vector growth causes iterator invalidation) | **100% Safe** (Parent frames remain immutable in memory)             |
| **Telemetry & Observability** | Poor (Historical context wiped on replan)               | Moderate (Difficult to trace dynamic mutations in deep trees)  | Very Poor (Mutates original sequence in-place)             | **Excellent** (Clean stack-trace isolation, historical auditability) |

---
## 3. State Drift Remediation: Dual-Stage Validation Engine

### The State Drift Problem
State drift occurs when a remediation sub-queue resolves a local blockage (e.g., closing a popup or scrolling a view) but introduces environmental side effects (e.g., loss of widget focus, coordinate shifts, or active window switches), invalidating the environmental assumptions of the paused parent queue upon return.
### Execution & Verification Workflow

```
[ Active Sub-Queue Executing ]
		│
		▼
[ Stage 1: Sub-Queue Post-Condition Verification ]
		│
┌───────┴───────┐
(Passed) (Failed)
│               │
▼               ▼
[frame.pop()] [Retry Sub-Queue / Escalate to Model]
		│
		▼
[ Stage 2: Parent Pre-Condition Sanity Check ]
		│
┌───────┴───────┐
(Passed) (Failed: State Drift Detected)
│               │
▼               ▼
[ Resume Parent ] [ Unwind Entire Stack & Trigger Full Replanning ]
```
### Core Engine Safeguards
1. **Sub-Queue Post-Condition Verification:** Confirms that the remediation action achieved its intended outcome (e.g., verifying that a modal has unmounted from the visual tree).
2. **Parent Pre-Condition Sanity Check:** Prior to resuming the parent queue, a lightweight assertion verifies critical invariants (e.g., verifying the original target window remains focused and visual bounding boxes remain within tolerance).
3. **Recovery Strategy:**
   * **Nominal:** The parent frame resumes deterministic execution without overhead.
   * **State Drift Fallback:** If pre-conditions fail, the engine aborts execution, clears the call frame stack, captures an updated perception snapshot (UI Automation tree and screenshot), and requests a full plan re-synthesis from the LLM.

# Orchestrator 
Orchestrator is the core of the project. It connects and orchestrates different parts like observation, sending data to llm, parsing, and executing to achieve the goal of this project. Hence, designing the architecture of this module is important step and challenging.

Targeted Life Cycle Flow:
```
[UI Input]
 │ 
 ▼ 
[Gateway]
 │ 
 ▼ 
[Orchestrator::HandleUserPrompt(text)] 
 │ 
 ├─► 1. Status = Observing ──► WorldStateBuilder.Build() ──► Current WorldState
 │ 
 ├─► 2. Append User Message + WorldState into currentChat 
 │ 
 ├─► 3. Status = Thinking ──► LlmSender.Send(currentChat) ──► Raw LLM Output 
 │ 
 ├─► 4. Parser.Parse(rawOutput) ──► ActionQueue (FIFO) 
 │ 
 ├─► 5. Status = Executing  
 │ 
 ▼ 
(Loop over ExecutionCallStack while Token not Cancelled) 
 │ 
 ├─► Dispatch Action via ActionDispatcher 
 │ 
 ├─► Check ActionResult 
 │  
 ├─► Success: Continue 
 │ 
 ├─► Fail: Trigger Replanning (send error back to LLM) 
 │ 
 └─► Requires Re-Observation: Break loop & go to Step 1 
 │ 
 ▼ 
[Sending Data to Ui via Gateway]
```

Issue: Handling Cancellation & Emergency Stop
- Problem:
	- Stopping an agent while it is working can be risky. If we kill the process in the middle of an action, the computer can get stuck in a bad state (like a mouse button staying pressed down or a file being only half-written). At the same time, if we wait for a long network request to finish, the app will feel frozen and unresponsive to the user.
- Provided Solution:
	- **Checkpoints Between Steps:** We use an `std::atomic<bool> cancelRequested` flag. Instead of killing code randomly, the Orchestrator checks this flag at safe checkpoints: before taking a screenshot, before sending data to the LLM, after receiving the response, and right before running any action.
	- **Clearing the Execution Stack:** When the user clicks stop or an error happens (`requestStop()` or `abortWorkflow()`), the Orchestrator immediately calls `activeCallStack->abort()`. This clears the remaining actions and resets
	- **Clean Thread Cleanup:** In the Orchestrator destructor, we call `requestStop()` and `waitForActiveTask()`. This waits for the background worker thread (`activeWorker.wait()`) to finish safely before closing the program

Issue: Handling Approval for critical actions (Human-in-the-Loop)
- Problem:
	- Some actions are dangerous, like deleting files, restarting the PC, or running terminal commands. The agent must not run these without asking the user first. However, we cannot put UI popups inside the low-level dispatcher or freeze the thread while waiting for an answer
- Provided Solution:
	- We designed a Class called `PermissionValidator` to act as filter between Planning and Dispatching in the flow. Before executing, `executeNextActionAsync()` pops the action into an `std::optional<ActionItem> pendingAction` The Orchestrator runs `PermissionValidator::validate()`, and If the action needs confirmation, the status changes to `WaitingForApproval`, and it calls the `onApprovalRequested` callback to notify the UI. The function exits right away, so no thread is stuck waiting.
	- If the validator completely blocks an action (`PermissionLevel::Denied`), we drop it and call `triggerReplanningAsync()` so the LLM knows it was blocked and can try another way

Issue: Handling Context & Memory Pruner 
- **Problem:**
	- The API itself that we use to send data to the datacenter has no internal memory. This lack of memory reduces the LLM's output accuracy because it forgets the state, the current conditions, and the user's primary goal. Additionally, if the volume of data and content we send to the LLM via the API call becomes too large, the output quality gets weaker. However, this does not mean that "the larger the request becomes, the worse the model inherently gets." Rather, the issue is that as the context grows larger, the probability of performance degradation in certain tasks increases. What is the reason?
		1. **Important information gets lost in too much data:** The model has to search through thousands of tokens to find which part is actually relevant to the current decision.
		2. **Old information can get mixed up with the current state** (lost in the context).
		3. **Cost and latency also increase.**

![[PerformancePerPContext.png]]
- Provided Solution:
	- **Temporary Environment Data:** `WorldState` is not saved forever[cite: 7, 8]. When we observe the desktop, we use the data only for the current step and clear it right away with `consumeState()`
	- **Context Compression:** When the context gets too big, `compressContext()` takes the current text from `currentChat->getContextWindow()`, sends it to the LLM with a special compression prompt, and saves a short summary back into `currentChat
	- **Clean History Storage:** The `Message` class only stores user prompts, LLM text, and plans. It does not store raw screen images or trees, keeping the database light.`

#### Why did we Choose Event-Driven architecture for Orchestrator instead of Polling/Busy-Waiting?
- **No Wasted CPU:** Polling with `while` loops and `sleep()` uses CPU power for nothing and can make the app lag. With an event-driven design, the Orchestrator sits idle until an event arrives
- **Smooth UI Integration:** The UI runs on the main thread, while the Orchestrator does heavy work in the background. Using callbacks (`std::function`) like `onStatusChanged` and `onMessageReceived` lets the UI update easily without complicated thread locks.
- **Clean Waiting for User Input:** When waiting for user approval, the system does not run any active loops. It just changes its state to `WaitingForApproval` and waits for the UI to call `handleUserApproval()`

#### Micro-Batching & Dynamic Re-Evaluation Loop
- ##### **Problem: State Drift in OS Automation**
	- Running long sequences of actions (for instance, 5 to 10 steps in a row) in an operating system environment is very fragile. Desktop applications are dynamic, asynchronous, and non-deterministic:
		* Windows and UI elements can take unexpected time to load.
		* Layouts and coordinates can shift mid-execution.
		* Unplanned pop-ups or modal dialogs can hijack input focus.
	- If the model plans too many steps ahead, even a tiny mismatch in step 1 or 2 invalidates the rest of the plan, which can easily cause incorrect clicks or destructive behavior.
- ##### **Provided Solution**
	##### **1. Micro-Batching (1–3 Actions per Turn)**
	Instead of predicting an entire multi-step workflow in one go, the LLM is constrained to output only small **micro-batches** containing 1 to 3 atomic actions per turn. This keeps each planning cycle focused and limits the impact of unexpected environment changes.
	##### **2. Automatic End-of-Stack Observation**
	During output parsing in `processLlmResponse()`, whenever an execution queue is created, the Orchestrator automatically appends a trailing observation action to the very end of the stack
$$\text{Queue} = [\text{Action}_1, \text{Action}_2, \dots, \text{Observe}]$$
	When execution reaches this final action, the dispatcher returns `ActionStatus::TriggerObserve`. This cleanly pauses the execution loop and tells the Orchestrator to capture fresh ground-truth data from the OS (such as updated UI trees or screens) before making any further decisions.
	**3. Autonomous Continuation vs. Termination**
	In the subsequent cycle, the model receives the fresh observation alongside the task history and decides how to proceed
		* **Continuation:** If the goal is not yet met, the model plans another micro-batch (1–3 actions), and the execution loop continues seamlessly.
		* **Completion / User Interaction:** If the objective is complete, or if the model needs to ask the user a question (Human-in-the-Loop), it leaves the `steps` array empty (`[]`). When the Orchestrator sees an empty stack, it transitions directly to `AgentStatus::Idle` and triggers `onTaskCompleted()`.

#### Failure Recovery & Replanning Pipeline
- ##### Problem: Cascading Failures
	Failures are normal when automating computer tasks (for example, file access denied errors or terminal commands timing out). If one action fails and the system keeps running the rest of the plan, it causes cascading errors that can break the environment further.
- ##### Provided Solution
	##### 1. Local Retry Mechanism
	The system first tries to recover on its own: if an action fails, it will retry that same action up to 2 times (`MAX_ACTION_RETRIES = 2`) before giving up.
	##### 2. Full Stack Clearing (Fail-Fast)
	If the action still fails after two retries, the system stops blind execution and follows a strict Fail-Fast approach:
	* **Discarding Remaining Actions:** It calls `activeCallStack->clear()` to immediately drop all remaining actions in the current queue, preventing broken steps from running.
	* **Logging the Failure:** The exact failure reason is recorded in the current task log.
	* **Triggering Replanning:** The Orchestrator calls `triggerReplanningAsync()`. This runs a lightweight, targeted observation focused on the error and asks the LLM to generate an alternative plan.
#### Current Task History (`currentTaskHistory`)
- ##### Problem: Amnesia vs. Context Window Pollution
	LLMs are stateless: without history, they have no idea which previous steps succeeded or failed. On the other hand, adding every tiny mouse move, click, and technical log to the permanent chat history (`contextWindow` and database) wastes tokens, slows down responses, and distracts the model from the user's main goal.
- ##### Provided Solution: Ephemeral Scratchpad Pattern
	We use `currentTaskHistory` as a temporary scratchpad for the active task, stored directly inside the chat object:
	* **Step-by-Step Logging:** As each action runs, its success or failure message is appended to this variable.
	* **Dynamic Context Injection:** Inside `triggerThinkingAsync()`, this history is temporarily appended to the user prompt sent to the LLM, giving the model full visibility over its recent actions.
	* **Preserved During User Interactions:** When the agent pauses to ask the user a question (Human-in-the-Loop), this scratchpad is kept intact so the user's answer does not wipe out what has been done so far.
	* **Automatic Cleanup:** Only after the entire task completes successfully is `currentTaskHistory` reset to `""`, keeping the main chat memory clean and lightweight for future turns.

![OrchestratorSequenceDiagram](docs/assets/OrchestratorSequenceDiagram.png)

---
#### Architectural Decision Record (ADR): Result and Error Reporting Pipeline
When actions run inside the `ActionExecutorServices`, they produce outputs, standard errors, or failure messages. These results reach the `ActionDispatcher` and must eventually be fed back to the LLM so it knows what happened.
We considered two main approaches (and rejected another) to route these results:
#### Options Considered
##### 1. Explicit Return via Output Struct (`ActionResult` to Orchestrator)
In this approach, the `ActionDispatcher` returns a detailed struct (containing `status`, `output`, and `errorMessage`) to the `Orchestrator` after every action. The Orchestrator then forwards this payload to `WorldStateBuilderService` to update the context.
- **Pros:** Keeps a strict hierarchy, provides a very explicit data flow, and completely isolates the dispatcher from the perception layer.
- **Cons:** Turns the Orchestrator into a heavy middleman for large data payloads (long terminal logs, file contents, or shell outputs), making the execution loop more complex.
##### 2. In-Place Mutation on the Action Object
In this approach, actions are passed by reference (`&`), and a mutable `output` field inside the action struct is populated during execution so the Orchestrator can read it later.
- **Cons (Rejected):** This violates the **Command Pattern**. Action structs should only represent the user or LLM's initial "intent" and input parameters. Modifying them during execution corrupts audit histories and makes multi-threaded execution unreliable.
##### 3. Decoupling Control Flow and Data Flow via Event Sink (Selected Approach)
In this model, we separate who handles the execution flow from who collects the data:
- **Control Flow:** The `ActionDispatcher` returns only a lightweight status enum (`ActionStatus::Success`, `Failed`, or `TriggerObserve`) directly to the `Orchestrator`.
- **Data Flow:** Error messages, terminal outputs, and search results are sent straight to `WorldStateBuilderService` using a sink method (`pushActionResult`).
### Comparison Matrix

| **Criteria**                | Return to Orchestrator                  | Mutate Action Reference         | Selected: Event Sink                                 |
| --------------------------- | --------------------------------------- | ------------------------------- | ---------------------------------------------------- |
| **Orchestrator Complexity** | Medium (has to manage payloads)         | High (mixes state and tracking) | **Minimal** (focuses purely on control flow)         |
| **Command Pattern & OOP**   | Fully compliant                         | Violates the pattern            | **Fully compliant**                                  |
| **Memory & Data Movement**  | Copies payloads through multiple layers | Unclear overhead                | **Minimal** (pushes directly into a temporary queue) |
| **Coupling**                | Loose                                   | Tight                           | **Controlled** (data pushed via Sink)                |
### Why We Chose the Event Sink Approach
1. **Fail-Fast and Preventing Cascading Failures:**
    When the LLM provides a multi-step plan, an error on step one usually invalidates all following steps. With this design, the moment the Orchestrator receives `ActionStatus::Failed`, it halts the remaining queue immediately. It then triggers a replan cycle so the LLM can fix the error right away.
2. **Targeted Observation and Token Savings:**
    When an action fails, the Orchestrator can use specific `ObservationFlags` to tell the `WorldStateBuilderService` to skip heavy operations (like OCR or collecting the entire accessibility tree) and instead build a lightweight state focused specifically on the error message.
3. **Managing Temporary Data Lifecycles (No "Ghost Feedback"):**
    Compile errors, terminal outputs, and text diffs are short-lived. Storing them in a temporary queue inside `WorldStateBuilderService` ensures they are cleared immediately after being consumed for the next prompt (`consumeState()`). This prevents stale error messages from confusing the model in later turns ("ghost feedback").
4. **Handling Observation Requests Smoothly (`FAR` / `fileAnalyzeRequest`):**
    Actions that do not interact with the OS directly, but instead request perception data (like `Actions::FAR` to analyze a file), pass straight from the dispatcher into the perception pipeline without cluttering the Orchestrator's execution logic.
### Architectural Trade-offs
- **Accepting Controlled Coupling:**
    From a strict theoretical standpoint, having `ActionDispatcher` depend on `WorldStateBuilderService` connects actuation to perception. However, this connection is strictly limited to one simple entry point: `pushActionResult`. The dispatcher knows nothing about how prompts are built, how screenshots are taken, or how state is managed—it just acts as an event producer.
- **The Decision:** We accepted this lightweight coupling because it keeps the Orchestrator simple, speeds up error handling, and makes the whole execution pipeline easier to maintain.


![OrchesratorOverview](docs/assets/OrchesratorOverview.png)
![OrchestratorSOverview](docs/assets/OrchestratorSOverview.png)
## Post-Implementation Evaluation & Protocol Evolution
After finalizing the initial build of the agent, the system's real-world behavior diverged noticeably from our design expectations.
##### 1. Initial Bottlenecks with Free & Low-Tier Models
For testing the initial build, we relied on free-tier models hosted on OpenRouter. Due to the limited instruction-following capabilities of these models, we encountered several critical issues:
- **Inconsistent Schema Adherence:** The models frequently deviated from the output format specified in the system prompt, causing the custom parser to fail.
- **Elevated Latency:** The sequential reasoning and raw generation process took excessively long, compounding execution delays across multi-step tasks.
##### 2. Architectural Finding: Native Tool Calling vs. Custom Text Protocols
During debugging, we evaluated the contrast between our custom prompt-based parsing and the industry-standard **Native Tool Calling (Function Calling)** protocol supported by advanced LLM APIs:
- **How Native Tool Calling Works:** Models trained natively on tool use output structured function calls rather than arbitrary free text. While tool execution still requires an external round-trip for client-side actions (executing the action locally and returning the observation back via the `tool` role), native APIs enforce structured JSON decoding at the inference layer. This significantly reduces format hallucination and eliminates conversational boilerplate.
- **Our Custom Text-Based Protocol:** We initially designed a custom parsing protocol where the model emitted formatted strings detailing the plan and actions, which our engine parsed and executed sequentially.
##### 3. Trade-off Analysis: Custom Protocol vs. Native Function Calling
- **Advantages of Our Custom Protocol:**
    - **Universal Model Compatibility:** Many open-source or lightweight models lack native function-calling APIs. Because our architecture relies purely on string responses, any completion model can interface with the orchestrator.
    - **Protocol Autonomy:** Building our own dispatcher and observation-action loop provided deeper control over custom micro-batching and execution rollbacks.
- **Trade-offs and Limitations:**
    - **Fragility:** Without API-level schema enforcement (constrained decoding), low-cost models regularly generate malformed syntax that crashes string-based parsers.
    - **Token Overhead & Context Bloat:** Documenting custom action schemas entirely inside the system prompt inflates prompt size, driving up costs and slowing down processing compared to native tool representations.
    - **Multi-Turn Latency:** Manual re-prompting, status synchronization, and sequential state re-injection create avoidable round-trip overhead compared to optimized, native conversational loops.
##### 4. Lessons Learned & Future Protocol Roadmap
Building this project from scratch exposed us to fundamental engineering trade-offs regarding how agents interface with LLMs. Below is a post-mortem of our custom protocol and our roadmap for future iterations.
##### The Reality of Custom Text-Based Protocols
During our initial core development, we implemented a custom text-based parsing protocol where models output structured reasoning and action plans as raw text strings.
- **The Good:** This approach guaranteed **Universal Model Compatibility**. Because it relied purely on standard string completion rather than proprietary API features, any model—including lightweight or open-source local models lacking native tool-calling support—could interface with our orchestrator. It also forced us to deeply understand state tracking, command parsing, and error-recovery pipelines from first principles.
- **The Bad:** It introduced significant friction. Without API-level schema enforcement (constrained decoding), low-cost models frequently hallucinated syntax or broke formatting rules, causing the custom parser to fail. Furthermore, embedding action schemas directly into system prompts inflated token overhead and increased multi-turn latency due to manual re-prompting and state synchronization.
##### Clarifying the "Reasoning-Loop" Misconception
A critical insight gained during implementation involved how data flows during execution:
- In a local desktop agent, **no model can execute external actions or observe the live operating system directly inside its internal reasoning loop (hidden state).**
- Whether using a custom text protocol or native APIs, an external round-trip is always mandatory: the model outputs an intent, the client executes it locally against the OS (moving the mouse, writing a file, etc.), captures a fresh observation, and feeds it back to the model as a new conversational turn.
##### Strategic Pivot: Hybrid Protocol Architecture
Rather than discarding our custom protocol, our architectural roadmap transitions toward a **Hybrid Protocol Strategy**:
1. **Native Function Calling for Tier-1 APIs:** For cloud-hosted, capable models, we are integrating native tool-calling protocols to leverage constrained decoding, reduce token overhead, and eliminate parsing fragility.
2. **Custom Protocol as a Fallback Layer:** We are preserving our custom string-parsing pipeline as a **Fallback/Offline Mode**. This ensures that our agent can still operate seamlessly with offline open-source models or proprietary endpoints that lack robust native tool support, keeping our software architecture fully model-agnostic and resilient.

---

---
