/*
 * Copyright (c) 2018 2018- ¦ÌNEST Foundation, and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>

#include <fc/io/json.hpp>
#include <fc/io/stdio.hpp>
#include <fc/rpc/cli.hpp>
#include <fc/rpc/websocket_api.hpp>

#include <graphene/app/api.hpp>
#include <graphene/chain/config.hpp>
#include <graphene/egenesis/egenesis.hpp>
#include <graphene/mini_wallet/wallet.hpp>

#include <boost/program_options.hpp>

#include <fc/log/console_appender.hpp>
#include <fc/log/file_appender.hpp>
#include <fc/log/logger.hpp>
#include <fc/log/logger_config.hpp>

#include <graphene/utilities/git_revision.hpp>
#include <boost/version.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <websocketpp/version.hpp>

#ifdef WIN32
# include <signal.h>
#else
# include <csignal>
#endif

using namespace graphene::app;
using namespace graphene::chain;
using namespace graphene::utilities;
using namespace graphene::wallet;
using namespace std;
namespace bpo = boost::program_options;

int main( int argc, char** argv )
{
   try {

      boost::program_options::options_description opts;
      opts.add_options()
         ("help,h", "Print this help message and exit.")
         ("server-rpc-endpoint,s", bpo::value<string>()->implicit_value("ws://127.0.0.1:8090"), "Server websocket RPC endpoint")
         ("server-rpc-user,u", bpo::value<string>(), "Server Username")
         ("server-rpc-password,p", bpo::value<string>(), "Server Password")
         ("daemon,d", "Run the wallet in daemon mode")
         ("wallet-file,w", bpo::value<string>()->implicit_value("wallet.json"), "wallet to load")
         ("wallet-password", bpo::value<string>(), "Password to lock the wallet file")
         ("chain-id", bpo::value<string>(), "chain ID to connect to")
         ("suggest-brain-key", "Suggest a safe brain key to use for creating your account")
         ("create-account,c", bpo::value<string>(), "Create an account")
         ("account-name", bpo::value<string>(), "Account name to create or for action")
         ("hash-sha256", bpo::value<string>(), "Generate SHA 256 hash from a string")
         ("generate-keys", bpo::value<int>()->implicit_value(3), "Generate a brain key and its derived public/private key pairs")
         ("import-key", bpo::value<string>(), "Import a private key for an account")
         ("import-keys", bpo::value<string>()->composing(), "Import private keys for an account; i.e. import_keys [\"key1\",\"key2\"] ...")
         ("send-message", bpo::value<string>(), "Send an SHA256 hash of message from an account to another")
         ("from-account,f", bpo::value<string>(), "Account name to act and pay")
         ("to-account,t", bpo::value<string>(), "Account name to receive")
         // ("memo", bpo::value<string>(), "The content which SHA256 hash will be sent")
         ("version,v", "Display version information");

      bpo::variables_map options;

      bpo::store( bpo::parse_command_line(argc, argv, opts), options );

      if( options.count("help") )
      {
         cout << opts << "\n";
         return 0;
      }
      if( options.count("version") )
      {
         cout << "Version: " << git_revision_description << "\n";
         cout << "SHA: " << git_revision_sha << "\n";
         cout << "Timestamp: " << fc::get_approximate_relative_time_string(fc::time_point_sec(git_revision_unix_timestamp)) << "\n";
         cout << "SSL: " << OPENSSL_VERSION_TEXT << "\n";
         cout << "Boost: " << boost::replace_all_copy(std::string(BOOST_LIB_VERSION), "_", ".") << "\n";
         cout << "Websocket++: " << websocketpp::major_version << "." << websocketpp::minor_version << "." << websocketpp::patch_version << "\n";
         return 0;
      }
      if( options.count("suggest-brain-key") )
      {
         auto keyinfo = utility::suggest_brain_key();
         string data = fc::json::to_pretty_string( keyinfo );
         std::cout << data.c_str() << std::endl;
         return 0;
      }
      if(options.count("hash-sha256"))
      {
         string txt = options.at("hash-sha256").as<string>();
         auto hash = fc::sha256::hash(txt); 
         string data = hash.str();
         cout << data.c_str() << endl;

         return 0;
      }
      if (options.count("generate-keys"))
      {
         auto keyinfo = utility::suggest_brain_key();

         int num = options.at("generate-keys").as<int>();
         if (num < 3)
            num = 3;
         auto keys = utility::derive_owner_keys_from_brain_key(keyinfo.brain_priv_key, num);

         cout << "brain_priv_key: \"" << keys[0].brain_priv_key << "\"" << endl;

         for (int ii = 0; ii < num; ++ii)
         {
            const char* key_name = ii == 0 ? "      owner_key:\"" : "  wif_priv_key: \"";
            cout << key_name << keys[ii].wif_priv_key << "\"" << endl;
            cout << "       pub_key: \"" << std::string(keys[ii].pub_key) << "\"" << endl;
            cout << endl;
         } 
         return 0;
      }

      fc::path data_dir;
      fc::logging_config cfg;
      fc::path log_dir = data_dir / "logs";

      fc::file_appender::config ac;
      ac.filename             = log_dir / "rpc" / "rpc.log";
      ac.flush                = true;
      ac.rotate               = true;
      ac.rotation_interval    = fc::hours( 1 );
      ac.rotation_limit       = fc::days( 1 );

      std::cout << "Logging RPC to file: " << (data_dir / ac.filename).preferred_string() << "\n";

      cfg.appenders.push_back(fc::appender_config( "default", "console", fc::variant(fc::console_appender::config(), 20)));
      cfg.appenders.push_back(fc::appender_config( "rpc", "file", fc::variant(ac, 5)));

      cfg.loggers = { fc::logger_config("default"), fc::logger_config( "rpc") };
      cfg.loggers.front().level = fc::log_level::info;
      cfg.loggers.front().appenders = {"default"};
      cfg.loggers.back().level = fc::log_level::debug;
      cfg.loggers.back().appenders = {"rpc"};

      // TODO:  We read wallet_data twice, once in main() to grab the
      //    socket info, again in wallet_api when we do
      //    load_wallet_file().  Seems like this could be better
      //    designed.

      wallet_data wdata;

      fc::path wallet_file( options.count("wallet-file") ? options.at("wallet-file").as<string>() : "wallet.json");
      if( fc::exists( wallet_file ) )
      {
         wdata = fc::json::from_file( wallet_file ).as<wallet_data>( GRAPHENE_MAX_NESTED_OBJECTS );
         if( options.count("chain-id") )
         {
            // the --chain-id on the CLI must match the chain ID embedded in the wallet file
            if( chain_id_type(options.at("chain-id").as<std::string>()) != wdata.chain_id )
            {
               std::cout << "Chain ID in wallet file does not match specified chain ID\n";
               return 1;
            }
         }
      }
      else
      {
         if( options.count("chain-id") )
         {
            wdata.chain_id = chain_id_type(options.at("chain-id").as<std::string>());
            std::cout << "Starting a new wallet with chain ID " << wdata.chain_id.str() << " (from CLI)\n";
         }
         else
         {
            wdata.chain_id = graphene::egenesis::get_egenesis_chain_id();
            std::cout << "Starting a new wallet with chain ID " << wdata.chain_id.str() << " (from egenesis)\n";
         }
      }

      // but allow CLI to override
      if( options.count("server-rpc-endpoint") )
         wdata.ws_server = options.at("server-rpc-endpoint").as<std::string>();
      if( options.count("server-rpc-user") )
         wdata.ws_user = options.at("server-rpc-user").as<std::string>();
      if( options.count("server-rpc-password") )
         wdata.ws_password = options.at("server-rpc-password").as<std::string>();

      fc::http::websocket_client client;
      idump((wdata.ws_server));
      auto con  = client.connect( wdata.ws_server );
      auto apic = std::make_shared<fc::rpc::websocket_api_connection>(*con, GRAPHENE_MAX_NESTED_OBJECTS);

      auto remote_api = apic->get_remote_api< login_api >(1);
      edump((wdata.ws_user)(wdata.ws_password) );
      FC_ASSERT( remote_api->login( wdata.ws_user, wdata.ws_password ), "Failed to log in to API server" );

      auto wapiptr = std::make_shared<wallet_api>( wdata, remote_api );
      wapiptr->set_wallet_filename( wallet_file.generic_string() );
      wapiptr->load_wallet_file();

      fc::api<wallet_api> wapi(wapiptr);

      auto wallet_cli = std::make_shared<fc::rpc::cli>( GRAPHENE_MAX_NESTED_OBJECTS );
      for( auto& name_formatter : wapiptr->get_result_formatters() )
         wallet_cli->format_result( name_formatter.first, name_formatter.second );

      boost::signals2::scoped_connection closed_connection(con->closed.connect([wallet_cli]{
         cerr << "Server has disconnected us.\n";
         exit(0);
      }));

      string pwd = "miuNESTcore";
      if (options.count("wallet-password"))
         pwd = options.at("wallet-password").as<string>();

      if( wapiptr->is_new() )
      {
         wapiptr->set_password(pwd);
         //std::cout << "Please use the set_password method to initialize a new wallet before continuing\n";
         //wallet_cli->set_prompt( "new >>> " );
      } else
         wallet_cli->set_prompt( "locked >>> " );

      boost::signals2::scoped_connection locked_connection(wapiptr->lock_changed.connect([&](bool locked) {
         wallet_cli->set_prompt(  locked ? "locked >>> " : "unlocked >>> " );
      }));

      wapiptr->unlock(pwd);

      if (options.count("import-key"))
      {
         string privKey = options.at("import-key").as<string>();
         string account = options.at("account-name").as<string>();
         auto res = wapiptr->import_key(account, privKey);
         const char* output = res ? "Import private key succeeded!" : "Import private key failed!";
         cout << output << endl;
         return 0;
      }
      if (options.count("import-keys"))
      {
         string keystr = options.at("import-keys").as<string>();
         auto keys = fc::json::from_string(keystr).as<vector<string>>(2);
         string account = options.at("account-name").as<string>();
         for (auto k : keys)
         {
            auto res = wapiptr->import_key(account, k);
            const char* output = res ? "successfully imported!" : "failed to import!";
            cout << "Private key: \"" << k << "\" " << endl;
         }
         return 0;
      }

      if (options.count("create-account"))
      {
         if (wapiptr->is_locked())
            return 1;

         auto keyinfo = utility::suggest_brain_key();
         string data = fc::json::to_pretty_string(keyinfo);
         cout << data.c_str() << endl;

         string name;
         string registrar="nathan";
         string referrer = "nathan";
         if (options.count("create-account"))
            name = options.at("create-account").as<string>();

         signed_transaction trans = wapiptr->create_account_with_brain_key(keyinfo.brain_priv_key, name, registrar, referrer, true);
         return 1;
      }
      if (options.count("send-message"))
      {
         if (wapiptr->is_locked())
         {
            cout << "Wallet is locked, please specifie wallet-password in the command line." << endl;
            return 1;
         }

         string from;
         string to;
         string memo; 

         if (options.count("from-account"))
            from = options.at("from-account").as<string>();
         if (options.count("to-account"))
            to = options.at("to-account").as<string>();

         memo = options.at("send-message").as<string>();
         memo = fc::sha256::hash(memo).str();

         signed_transaction trans = wapiptr->send_message(from, to, memo, true);
         return 1;
      }

      if( !options.count( "daemon" ) )
      {
         wallet_cli->register_api( wapi );
         wallet_cli->start();
         wallet_cli->wait();
      }
      else
      {
      }

      wapi->save_wallet_file(wallet_file.generic_string());
   }
   catch ( const fc::exception& e )
   {
      std::cout << e.to_detail_string() << "\n";
      return -1;
   }
   return 0;
}
