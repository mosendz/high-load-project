#ifndef matches_h
#define matches_h

#include <cassandra.h>
#include <cppcms/application.h>
#include <cppcms/service.h>

class matches : public cppcms::application
{
public:
    matches(cppcms::service &srv);
    void getMatches();
    void getMatch(std::string id);
    void editMatch(std::string id);

private:
    CassCluster *cluster;
    CassSession *session;
    CassFuture *connect_future;
};

#endif