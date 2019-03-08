/*
* Copyright (c) 2015 Cryptonomex, Inc., and contributors.
* Copyright (c) 2018- ��NEST Foundation, and contributors.
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

#pragma once

#include <graphene/db/object.hpp>
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>
//#include <graphene/chain/protocol/asset.hpp>
#include <fc/crypto/sha256.hpp>
#include <string>

#define CREATE_WATCH_DOG_FEE 5000


namespace graphene { namespace chain {

	struct create_watch_dog_operation : public base_operation
	{
		struct fee_parameters_type
		{
			uint64_t fee = CREATE_WATCH_DOG_FEE * GRAPHENE_BLOCKCHAIN_PRECISION;
		};

		asset fee;
		account_id_type watch_account;
		account_id_type inherit_account;
		fc::sha256      answer_hash;

		account_id_type fee_payer() const { return watch_account; }
		void            validate() const {};
		share_type      calculate_fee(const fee_parameters_type& k) const { return k.fee; };
	};

	struct prepare_watch_dog_operation : public base_operation
	{
		struct fee_parameters_type {uint64_t fee = 0;};

		asset fee;
		account_id_type  account;
		account_id_type  nathan_account;

		account_id_type fee_payer() const { return nathan_account; }
		void            validate() const {};
		share_type      calculate_fee(const fee_parameters_type& k) const { return 0; }
	};

	struct remind_account_operation : public base_operation
	{
		struct fee_parameters_type {uint64_t fee = 0;};

		asset fee;
		account_id_type  account;
		account_id_type  nathan_account;

		account_id_type fee_payer() const { return nathan_account; }
		void            validate() const {};
		share_type      calculate_fee(const fee_parameters_type& k) const { return 0; }
	};

	struct answer_watch_dog_operation : public base_operation
	{
		struct fee_parameters_type {uint64_t fee = 0;};

		asset fee;
		account_id_type   account;
		fc::sha256        answer_hash;

		account_id_type fee_payer() const { return account; }
		void            validate() const {};
		share_type      calculate_fee(const fee_parameters_type& k) const { return 0; }
	};

	struct start_account_recover_operation : public base_operation
	{
		struct fee_parameters_type {uint64_t fee = 0;};

		asset fee;
		account_id_type  account;
		account_id_type  nathan_account;

		account_id_type fee_payer() const { return nathan_account; }
		void            validate() const {};
		share_type      calculate_fee(const fee_parameters_type& k) const { return 0; }
	};

	struct transfer_lost_asset_operation : public base_operation
	{
		struct fee_parameters_type {uint64_t fee = 0;};

		asset fee;
		account_id_type  account;
		account_id_type  nathan_account;

		account_id_type fee_payer() const { return nathan_account; }
		void            validate() const {};
		share_type      calculate_fee(const fee_parameters_type& k) const { return 0; }
	};

	struct delete_watch_dog_operation : public base_operation
	{
		struct fee_parameters_type {uint64_t fee = 0;};

		asset fee;
		account_id_type  account;
		account_id_type  nathan_account;

		account_id_type fee_payer() const { return nathan_account; }
		void            validate() const {};
		share_type      calculate_fee(const fee_parameters_type& k) const { return 0; }
	};

} }

FC_REFLECT(graphene::chain::create_watch_dog_operation::fee_parameters_type, (fee))
FC_REFLECT(graphene::chain::prepare_watch_dog_operation::fee_parameters_type, (fee))
FC_REFLECT(graphene::chain::remind_account_operation::fee_parameters_type, (fee))
FC_REFLECT(graphene::chain::answer_watch_dog_operation::fee_parameters_type, (fee))
FC_REFLECT(graphene::chain::start_account_recover_operation::fee_parameters_type, (fee))
FC_REFLECT(graphene::chain::transfer_lost_asset_operation::fee_parameters_type, (fee))
FC_REFLECT(graphene::chain::delete_watch_dog_operation::fee_parameters_type, (fee))

FC_REFLECT(graphene::chain::create_watch_dog_operation, (fee)(watch_account)(inherit_account)(answer_hash))
FC_REFLECT(graphene::chain::prepare_watch_dog_operation, (fee)(account)(nathan_account))
FC_REFLECT(graphene::chain::remind_account_operation, (fee)(account)(nathan_account))
FC_REFLECT(graphene::chain::answer_watch_dog_operation, (fee)(account)(answer_hash))
FC_REFLECT(graphene::chain::start_account_recover_operation, (fee)(account)(nathan_account))
FC_REFLECT(graphene::chain::transfer_lost_asset_operation, (fee)(account)(nathan_account))
FC_REFLECT(graphene::chain::delete_watch_dog_operation, (fee)(account)(nathan_account))