#include "wasm_contract.hpp"
#include <graphene/chain/account_evaluator.hpp>
#include "wasm_interface.hpp"

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

	wasm_interface wasmif(wavm);

	digest_type   code_id;
	shared_string code;
	apply_context context;

	wasmif.apply(code_id, code, context);

	return "";
}