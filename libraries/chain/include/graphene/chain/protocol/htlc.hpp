/*
* Copyright (c) 2015 Cryptonomex, Inc., and contributors.
* Copyright (c) 2018- ¦ÌNEST Foundation, and contributors.
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
#include <graphene/chain/protocol/asset.hpp>
#include <fc/time.hpp>
#include <vector>

#define GRAPHENE_HTLC_PREPARE_FEE              10
#define GRAPHENE_HTLC_DAILY_FEE                1
#define GRAPHENE_HTLC_REDEEM_FEE               2
#define GRAPHENE_HTLC_KB_FEE                   1
#define GRAPHENE_HTLC_EXTEND_EXPIRY_FEE        5
#define GRAPHENE_HTLC_MAXIMUM_DURRATION        9999999999
#define GRAPHENE_HTLC_MAXIMUM_PREIMAGE_LENGTH  128

namespace graphene { namespace chain {

	struct htlc_prepare_operation : public base_operation
	{
		struct fee_parameters_type
		{
			uint64_t fee       = GRAPHENE_HTLC_PREPARE_FEE   * GRAPHENE_BLOCKCHAIN_PRECISION;
			uint64_t daily_fee = GRAPHENE_HTLC_DAILY_FEE     * GRAPHENE_BLOCKCHAIN_PRECISION;
		};

		asset                 fee;
		account_id_type       depositor;
		account_id_type       recipient;
		asset                 amount;

		std::string           hash_algorithm; // "sha256", "sha1", "ripemd160"
		std::string           preimage_hash;
		uint16_t              preimage_length;
		fc::time_point_sec    timeout_threshold;
		extensions_type       extensions;

		account_id_type fee_payer() const { return depositor; }
		void            validate() const;
		share_type      calculate_fee(const fee_parameters_type& k) const;
	};

	struct htlc_redeem_operation : public base_operation
	{
		struct fee_parameters_type
		{
			uint64_t fee    = GRAPHENE_HTLC_REDEEM_FEE * GRAPHENE_BLOCKCHAIN_PRECISION;
			uint64_t kb_fee = GRAPHENE_HTLC_KB_FEE     * GRAPHENE_BLOCKCHAIN_PRECISION;
		};

		asset                 fee;
		account_id_type       fee_paying_account;
		account_id_type       depositor;
		account_id_type       recipient;
		htlc_id_type          htlc_id;
		std::string           preimage;
		extensions_type       extensions;

		account_id_type fee_payer() const { return fee_paying_account; }
		void            validate() const;
		share_type      calculate_fee(const fee_parameters_type& k) const;
	};

	struct htlc_extend_expiry_operation : public base_operation
	{
		struct fee_parameters_type
		{
			uint64_t fee       = GRAPHENE_HTLC_EXTEND_EXPIRY_FEE * GRAPHENE_BLOCKCHAIN_PRECISION;
			uint64_t daily_fee = GRAPHENE_HTLC_DAILY_FEE         * GRAPHENE_BLOCKCHAIN_PRECISION;
		};

		asset              fee;
		account_id_type    depositor;
		htlc_id_type       htlc_id;
		fc::time_point_sec timeout_threshold;
		extensions_type    extensions;

		account_id_type fee_payer() const { return depositor; }
		void            validate() const;
		share_type      calculate_fee(const fee_parameters_type& k) const;
	};

	struct htlc_refund_operation : public base_operation
	{
		struct fee_parameters_type { uint64_t fee = 0; };

		asset           fee;
		account_id_type depositor;
		account_id_type nathan_account;
		asset           amount;
		htlc_id_type    htlc_id;
		extensions_type extensions;

		account_id_type fee_payer() const { return nathan_account; }
		void            validate() const;
		share_type      calculate_fee(const fee_parameters_type& k) const { return 0; }
	};

} } // graphene::chain

FC_REFLECT(graphene::chain::htlc_prepare_operation::fee_parameters_type, (fee)(daily_fee))
FC_REFLECT(graphene::chain::htlc_redeem_operation::fee_parameters_type, (fee)(kb_fee))
FC_REFLECT(graphene::chain::htlc_extend_expiry_operation::fee_parameters_type, (fee)(daily_fee))
FC_REFLECT(graphene::chain::htlc_refund_operation::fee_parameters_type, (fee))

FC_REFLECT(graphene::chain::htlc_prepare_operation, (fee)(depositor)(recipient)(amount)(hash_algorithm)(preimage_hash)(preimage_length)(timeout_threshold)(extensions))
FC_REFLECT(graphene::chain::htlc_redeem_operation, (fee)(fee_paying_account)(depositor)(recipient)(htlc_id)(preimage)(extensions))
FC_REFLECT(graphene::chain::htlc_extend_expiry_operation, (fee)(depositor)(htlc_id)(timeout_threshold)(extensions))
FC_REFLECT(graphene::chain::htlc_refund_operation, (fee)(depositor)(nathan_account)(amount)(htlc_id)(extensions))
