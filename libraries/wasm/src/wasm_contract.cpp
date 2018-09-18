#include "wasm_contract.hpp"
#include <graphene/chain/account_evaluator.hpp>
#include "wasm_interface.hpp"

using eosio::chain::wasm_interface;
using eosio::chain::apply_context;
using eosio::chain::transaction_context;
using eosio::chain::action;

namespace graphene {
namespace chain {

string smart_contract_deploy_evaluator::construct_smart_contract(const string &bytecode,
																const contract_addr_type &contract_addr,
																const string &construct_data,
																const string &abi_json)
{
	//TODO:
	return "";
}


string smart_contract_call_evaluator::call_smart_contract(const string &bytecode,
														   const contract_addr_type &contract_addr,
														   const string &call_data,
														   const string &abi_json,
														   const string &starting_state)
{
	//TODO

	wasm_interface wasmif(wasm_interface::vm_type::wavm);

	transaction_context trx_context;
	action              act;
	account_name        receiver;
	apply_context       context(trx_context, act, receiver);
	digest_type         code_id;
	shared_string       code;

	wasmif.apply(code_id, code, context);

	return "";
}

}
}