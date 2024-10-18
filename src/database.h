#pragma once
#include "database_addition_struct.h"
#include "db_connector.h"

namespace db {
    using namespace std::literals;
    using pqxx::operator"" _zv;

    class Database {
    public:
        Database(const DBSetting& dbs) :
            cp_{ dbs.num_connections, [url = std::move(dbs.url)]{
            auto conn = std::make_shared<pqxx::connection>(url);
            conn->prepare("select_one", "SELECT 1;");
            return conn; } }
        {
            auto conn = cp_.GetConnection();
            pqxx::work work{ *conn };

            work.exec(R"(
            CREATE TABLE IF NOT EXISTS retired_players (
                id UUID CONSTRAINT id_player PRIMARY KEY,
                name varchar(100),
                score integer,
                play_time_ms integer
            );
            )"_zv);
            
            work.exec(R"(
            CREATE INDEX IF NOT EXISTS retired_players_play_time_ms_name_idx ON retired_players (score DESC, play_time_ms, name);
                )"_zv);

            work.commit();
        }

            void WriteToBD(const GameRecords& gr);
            const std::vector<GameRecords> GetRecords(int offset, int max_elem);


    private:
        db_conn::ConnectionPool cp_;
    };
}
