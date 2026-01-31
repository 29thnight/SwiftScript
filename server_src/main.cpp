#include "stdafx.h"
#include "lsp_connection.hpp"
#include "lsp_server.hpp"

int main() {
    LspServer server;

    JsonRpcConnection conn([&](const std::string& raw, JsonRpcConnection& connection) {
        bool hasResp = false;
        std::string resp = server.Handle(raw, hasResp);
        if (hasResp && !resp.empty()) {
            connection.Send(resp);
        }

        // flush notifications
        std::string out;
        while (server.PopOutgoing(out)) {
            connection.Send(out);
        }
    });

    conn.Run();
    return 0;
}
