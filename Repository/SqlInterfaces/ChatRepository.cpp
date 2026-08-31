#include "ChatRepository.h"
#include "../DbMappers.h"
#include <sqlite3.h>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace agent::storage {

    ChatRepository::ChatRepository(std::shared_ptr<DatabaseManager> dbManager)
        : m_dbManager(std::move(dbManager)) {}

    int64_t ChatRepository::getCurrentTimeSec() {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

    std::vector<ChatSummary> ChatRepository::getChatSummaries() {
        std::vector<ChatSummary> summaries;
        const char* sql = R"(
            SELECT c.id, c.title, c.updated_at, COUNT(m.id) as msg_count
            FROM chats c
            LEFT JOIN messages m ON c.id = m.chat_id
            GROUP BY c.id
            ORDER BY c.updated_at DESC;
        )";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(m_dbManager->getConnection(), sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                ChatSummary summary;
                const unsigned char* idText = sqlite3_column_text(stmt, 0);
                const unsigned char* titleText = sqlite3_column_text(stmt, 1);

                summary.id = idText ? reinterpret_cast<const char*>(idText) : "";
                summary.title = titleText ? reinterpret_cast<const char*>(titleText) : "";
                summary.lastUpdatedUnixSec = sqlite3_column_int64(stmt, 2);
                summary.messageCount = static_cast<size_t>(sqlite3_column_int(stmt, 3));
                summaries.push_back(std::move(summary));
            }
            sqlite3_finalize(stmt);
        }
        return summaries;
    }

    std::optional<chat::ChatHistory> ChatRepository::loadChat(const std::string& chatId) {
        const char* sqlChat = "SELECT title, plan_json FROM chats WHERE id = ?;";
        sqlite3_stmt* stmtChat = nullptr;

        if (sqlite3_prepare_v2(m_dbManager->getConnection(), sqlChat, -1, &stmtChat, nullptr) != SQLITE_OK) {
            return std::nullopt;
        }

        sqlite3_bind_text(stmtChat, 1, chatId.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmtChat) != SQLITE_ROW) {
            sqlite3_finalize(stmtChat);
            return std::nullopt; // Chat not found
        }

        // 1. Construct base ChatHistory
        const unsigned char* rawTitle = sqlite3_column_text(stmtChat, 0);
        std::string title = rawTitle ? reinterpret_cast<const char*>(rawTitle) : "";

        chat::ChatHistory history(chatId, title);

        // 2. Load and deserialize Plan
        const char* planText = reinterpret_cast<const char*>(sqlite3_column_text(stmtChat, 1));
        if (planText != nullptr) {
            std::string planStr(planText);
            history.updatePlan(mappers::deserializePlan(planStr));
        }
        sqlite3_finalize(stmtChat);

        // 3. Load all Messages for this chat
        const char* sqlMsgs = "SELECT id, user_input, llm_result, timestamp_sec FROM messages WHERE chat_id = ? ORDER BY timestamp_sec ASC;";
        sqlite3_stmt* stmtMsgs = nullptr;

        if (sqlite3_prepare_v2(m_dbManager->getConnection(), sqlMsgs, -1, &stmtMsgs, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmtMsgs, 1, chatId.c_str(), -1, SQLITE_TRANSIENT);

            while (sqlite3_step(stmtMsgs) == SQLITE_ROW) {
                const unsigned char* rawMsgId = sqlite3_column_text(stmtMsgs, 0);
                const unsigned char* rawUserInput = sqlite3_column_text(stmtMsgs, 1);
                const unsigned char* rawLlmRes = sqlite3_column_text(stmtMsgs, 2);

                std::string msgId = rawMsgId ? reinterpret_cast<const char*>(rawMsgId) : "";
                std::string userInput = rawUserInput ? reinterpret_cast<const char*>(rawUserInput) : "";
                std::string llmResult = rawLlmRes ? reinterpret_cast<const char*>(rawLlmRes) : "";
                int64_t timestamp = sqlite3_column_int64(stmtMsgs, 3);

                chat::Message msg(msgId, userInput, llmResult, timestamp);
                history.addMessage(std::move(msg));
            }
            sqlite3_finalize(stmtMsgs);
        }

        return history;
    }

    bool ChatRepository::saveChat(const chat::ChatHistory& chat) {
        m_dbManager->beginTransaction();
        try {
            int64_t now = getCurrentTimeSec();

            // 1. Save Chat metadata & Plan
            const char* sqlChat = R"(
                INSERT INTO chats (id, title, plan_json, created_at, updated_at)
                VALUES (?, ?, ?, ?, ?)
                ON CONFLICT(id) DO UPDATE SET
                title=excluded.title, plan_json=excluded.plan_json, updated_at=excluded.updated_at;
            )";

            sqlite3_stmt* stmtChat = nullptr;
            if (sqlite3_prepare_v2(m_dbManager->getConnection(), sqlChat, -1, &stmtChat, nullptr) != SQLITE_OK) {
                throw std::runtime_error("Failed to prepare chat statement");
            }

            std::string planJson = mappers::serializePlan(chat.getPlan());

            sqlite3_bind_text(stmtChat, 1, chat.getId().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmtChat, 2, chat.getTitle().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmtChat, 3, planJson.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmtChat, 4, now); // created_at
            sqlite3_bind_int64(stmtChat, 5, now); // updated_at

            if (sqlite3_step(stmtChat) != SQLITE_DONE) {
                sqlite3_finalize(stmtChat);
                throw std::runtime_error("Failed to execute chat statement");
            }
            sqlite3_finalize(stmtChat);

            // 2. Save Messages
            const char* sqlMsg = R"(
                INSERT INTO messages (id, chat_id, user_input, llm_result, timestamp_sec)
                VALUES (?, ?, ?, ?, ?)
                ON CONFLICT(id) DO UPDATE SET
                user_input=excluded.user_input, llm_result=excluded.llm_result, timestamp_sec=excluded.timestamp_sec;
            )";

            sqlite3_stmt* stmtMsg = nullptr;
            if (sqlite3_prepare_v2(m_dbManager->getConnection(), sqlMsg, -1, &stmtMsg, nullptr) != SQLITE_OK) {
                throw std::runtime_error("Failed to prepare message statement");
            }

            for (const auto& msg : chat.getMessages()) {
                sqlite3_bind_text(stmtMsg, 1, msg.getId().c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmtMsg, 2, chat.getId().c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmtMsg, 3, msg.getUserInput().c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmtMsg, 4, msg.getLlmResult().c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int64(stmtMsg, 5, msg.getTimestampUnixSec());

                if (sqlite3_step(stmtMsg) != SQLITE_DONE) {
                    sqlite3_finalize(stmtMsg);
                    throw std::runtime_error("Failed to execute msg statement");
                }
                sqlite3_reset(stmtMsg);
            }
            sqlite3_finalize(stmtMsg);

            m_dbManager->commit();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "saveChat failed: " << e.what() << std::endl;
            m_dbManager->rollback();
            return false;
        }
    }

    bool ChatRepository::appendMessage(const std::string& chatId, const chat::Message& msg) {
        m_dbManager->beginTransaction();
        try {
            const char* sqlMsg = R"(
                INSERT INTO messages (id, chat_id, user_input, llm_result, timestamp_sec)
                VALUES (?, ?, ?, ?, ?);
            )";

            sqlite3_stmt* stmtMsg = nullptr;
            if (sqlite3_prepare_v2(m_dbManager->getConnection(), sqlMsg, -1, &stmtMsg, nullptr) != SQLITE_OK) {
                throw std::runtime_error("Failed to prepare append message statement");
            }

            sqlite3_bind_text(stmtMsg, 1, msg.getId().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmtMsg, 2, chatId.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmtMsg, 3, msg.getUserInput().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmtMsg, 4, msg.getLlmResult().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmtMsg, 5, msg.getTimestampUnixSec());

            if (sqlite3_step(stmtMsg) != SQLITE_DONE) {
                sqlite3_finalize(stmtMsg);
                throw std::runtime_error("Failed to step append message");
            }
            sqlite3_finalize(stmtMsg);

            // Update chat's updated_at timestamp
            const char* sqlUpdate = "UPDATE chats SET updated_at = ? WHERE id = ?;";
            sqlite3_stmt* stmtUpdate = nullptr;
            if (sqlite3_prepare_v2(m_dbManager->getConnection(), sqlUpdate, -1, &stmtUpdate, nullptr) == SQLITE_OK) {
                sqlite3_bind_int64(stmtUpdate, 1, getCurrentTimeSec());
                sqlite3_bind_text(stmtUpdate, 2, chatId.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_step(stmtUpdate);
                sqlite3_finalize(stmtUpdate);
            }

            m_dbManager->commit();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "appendMessage failed: " << e.what() << std::endl;
            m_dbManager->rollback();
            return false;
        }
    }

    bool ChatRepository::updatePlan(const std::string& chatId, const chat::Plan& plan) {
        const char* sql = "UPDATE chats SET plan_json = ?, updated_at = ? WHERE id = ?;";
        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(m_dbManager->getConnection(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        std::string planJson = mappers::serializePlan(plan);

        sqlite3_bind_text(stmt, 1, planJson.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, getCurrentTimeSec());
        sqlite3_bind_text(stmt, 3, chatId.c_str(), -1, SQLITE_TRANSIENT);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);

        return success;
    }

    bool ChatRepository::deleteChat(const std::string& chatId) {
        const char* sql = "DELETE FROM chats WHERE id = ?;";
        sqlite3_stmt* stmt = nullptr;
        
        if (sqlite3_prepare_v2(m_dbManager->getConnection(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_text(stmt, 1, chatId.c_str(), -1, SQLITE_TRANSIENT);
        
        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        
        return success;
    }

}