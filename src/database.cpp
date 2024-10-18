#include "database.h"


namespace db {
    void Database::WriteToBD(const GameRecords& gr) {
        using namespace std::literals;
        using pqxx::operator"" _zv;

        auto conn = cp_.GetConnection();
        pqxx::work work{ *conn };

        work.exec("START TRANSACTION"_zv);
        work.exec_params(
            R"(INSERT INTO retired_players (id, name, score, play_time_ms) VALUES ($1, $2, $3, $4);
        )"_zv, util::PlayerId::New().ToString(), gr.name, gr.score, gr.played_time);
        work.commit();
    }

    const std::vector<GameRecords> Database::GetRecords(int offset, int max_elem) {
        using namespace std::literals;
        using pqxx::operator"" _zv;

        std::vector<GameRecords> records;
        auto conn = cp_.GetConnection();
        pqxx::read_transaction tx{ *conn };

        tx.exec("START TRANSACTION"_zv);
        auto author_req = "SELECT name, score, play_time_ms FROM retired_players ORDER BY score DESC, play_time_ms, name LIMIT " + std::to_string(max_elem) + " OFFSET " + std::to_string(offset) + ";";

        for (auto [name, score, play_time_ms] : tx.query<std::string, size_t, size_t>(author_req)) {
            GameRecords gr{ name, score, static_cast<double>(play_time_ms) / 1000 };
            records.push_back(gr);
        }
        tx.commit();

        return records;
    }
}
