#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/http_request.h>
#include <cppcms/url_dispatcher.h>

#include <cassandra.h>
#include <string>
#include <fstream>
#include <sstream>

#include "matches.h"

matches::matches(cppcms::service &srv) : cppcms::application(srv)
{
    dispatcher().assign("/matches", &matches::getMatches, this);
    dispatcher().assign("/match/(\\d+)", &matches::getMatch, this, 1);

    cluster = cass_cluster_new();
    session = cass_session_new();
    printf("Connection started\n");
    cass_cluster_set_contact_points(cluster, "127.0.0.1");
    connect_future = cass_session_connect_keyspace(session, cluster, "my_matches");

    CassError rc = cass_future_error_code(connect_future);

    if (rc != CASS_OK)
    {
        const char *message;
        size_t message_length;
        cass_future_error_message(connect_future, &message, &message_length);
        fprintf(stderr, "Connect error: '%.*s'\n", (int)message_length, message);
    }
    else
    {
        printf("Connection successful\n");
    }
}

void matches::getMatches()
{
    CassStatement *statement = cass_statement_new("select * from matchlist", 0);

    CassFuture *query_future = cass_session_execute(session, statement);

    cass_statement_free(statement);

    CassError rc = cass_future_error_code(query_future);

    if (rc != CASS_OK)
    {
        printf("Query error: %s\n", cass_error_desc(rc));
        response().out() << cass_error_desc(rc);
        return;
    }

    const CassResult *result = cass_future_get_result(query_future);
    CassIterator *it = cass_iterator_from_result(result);
    cppcms::json::value col_obj;
    int i = 0;
    while (cass_iterator_next(it))
    {
        const CassRow *row = cass_iterator_get_row(it);
        const char *team1, *team2;
        int score1, score2, id;
        size_t size1, size2;
        cass_value_get_string(cass_row_get_column_by_name(row, "team1"), &team1, &size1);
        cass_value_get_string(cass_row_get_column_by_name(row, "team2"), &team2, &size2);
        cass_value_get_int32(cass_row_get_column_by_name(row, "score1"), &score1);
        cass_value_get_int32(cass_row_get_column_by_name(row, "score2"), &score2);
        cass_value_get_int32(cass_row_get_column_by_name(row, "id"), &id);
        cppcms::json::value match_obj;
        char uri[30];
        sprintf(uri, "%s%d", "/match/", id);
        match_obj["team1"] = team1;
        match_obj["team2"] = team2;
        match_obj["score1"] = score1;
        match_obj["score2"] = score2;
        match_obj["self"] = uri;
        col_obj["macthes"][i++] = match_obj;
    }
    cass_iterator_free(it);
    cass_future_free(query_future);
    response().out() << col_obj;
}

void matches::getMatch(std::string str)
{
    int id = 0;
    try
    {
        id = std::stoi(str);
    }
    catch (std::exception const &e)
    {
        response().out() << "Wrong id";
        return;
    }
    if (request().request_method() != "PUT")
    {
        CassStatement *statement = cass_statement_new("select * from matchlist where id = ?", 1);
        cass_statement_bind_int32(statement, 0, id);

        CassFuture *query_future = cass_session_execute(session, statement);

        cass_statement_free(statement);

        CassError rc = cass_future_error_code(query_future);

        if (rc != CASS_OK)
        {
            printf("Query error: %s\n", cass_error_desc(rc));
            response().out() << cass_error_desc(rc);
            return;
        }

        const CassResult *result = cass_future_get_result(query_future);
        int n = cass_result_row_count(result);
        if (n == 0)
        {
            response().out() << "No data";
        }
        const CassRow *row = cass_result_first_row(result);
        const char *team1, *team2;
        int score1, score2;
        size_t size1, size2;
        cass_value_get_string(cass_row_get_column_by_name(row, "team1"), &team1, &size1);
        cass_value_get_string(cass_row_get_column_by_name(row, "team2"), &team2, &size2);
        cass_value_get_int32(cass_row_get_column_by_name(row, "score1"), &score1);
        cass_value_get_int32(cass_row_get_column_by_name(row, "score2"), &score2);
        cass_value_get_int32(cass_row_get_column_by_name(row, "id"), &id);
        cppcms::json::value match_obj;
        char uri[30];
        sprintf(uri, "%s%d", "/match/", id);
        match_obj["team1"] = team1;
        match_obj["team2"] = team2;
        match_obj["score1"] = score1;
        match_obj["score2"] = score2;
        match_obj["self"] = uri;
        cass_future_free(query_future);
        cass_result_free(result);
        response().out() << match_obj;
    }
    else
    {
        std::pair<void *, size_t> body = request().raw_post_data();
        std::string content = std::string((const char *)body.first, body.second);
        std::istringstream in(content);
        cppcms::json::value match_obj;
        in >> match_obj;
        int score1 = match_obj.get<int>("score1");
        int score2 = match_obj.get<int>("score2");
        CassStatement *statement = cass_statement_new("update matchlist set score1 = ?, score2 = ? where id = ?", 3);
        cass_statement_bind_int32(statement, 0, score1);
        cass_statement_bind_int32(statement, 1, score2);
        cass_statement_bind_int32(statement, 2, id);
        CassFuture *query_future = cass_session_execute(session, statement);
        cass_statement_free(statement);
        CassError rc = cass_future_error_code(query_future);
        if (rc != CASS_OK)
        {
            printf("Query result: %s\n", cass_error_desc(rc));
        }
        response().out() << match_obj;
    }
}