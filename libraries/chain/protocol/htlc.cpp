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

#include <graphene/chain/protocol/htlc.hpp>

namespace graphene { namespace chain {

	void htlc_prepare_operation::validate() const
	{
		FC_ASSERT(fee.amount >= 0);
		FC_ASSERT(depositor != recipient);
		FC_ASSERT(amount.amount > 0);
		FC_ASSERT(timeout_threshold < fc::time_point::now() + fc::seconds(GRAPHENE_HTLC_MAXIMUM_DURRATION));
		FC_ASSERT(preimage_length <= GRAPHENE_HTLC_MAXIMUM_PREIMAGE_LENGTH);

		if ("sha256" == hash_algorithm)
			FC_ASSERT(preimage_hash.size() == 64, "preimage hash string does not match sha256");
		else if ("sha1" == hash_algorithm || "ripemd160" == hash_algorithm)
			FC_ASSERT(preimage_hash.size() == 40, "preimage hash string does not match sha1 or ripemd160");
		else
			FC_ASSERT(false, "unsupported hash algorithm");
	}

	share_type htlc_prepare_operation::calculate_fee(const fee_parameters_type& k) const
	{
		int64_t interval_days = (timeout_threshold - fc::time_point::now()).to_seconds() / (60 * 60 * 24);
		return k.fee + k.daily_fee * interval_days;
	}

	void htlc_redeem_operation::validate() const
	{
		FC_ASSERT(htlc_id.space_id == implementation_ids);
		FC_ASSERT(htlc_id.type_id == impl_htlc_object_type);
		FC_ASSERT(fee.amount >= 0);
	}

	share_type htlc_redeem_operation::calculate_fee(const fee_parameters_type& k) const
	{
		return k.fee + calculate_data_fee(fc::raw::pack_size(preimage), k.kb_fee);
	}

	void htlc_extend_expiry_operation::validate() const
	{
		FC_ASSERT(htlc_id.space_id == implementation_ids);
		FC_ASSERT(htlc_id.type_id == impl_htlc_object_type);
		FC_ASSERT(fee.amount >= 0);
		FC_ASSERT(timeout_threshold < fc::time_point::now() + fc::seconds(GRAPHENE_HTLC_MAXIMUM_DURRATION));
	}

	share_type htlc_extend_expiry_operation::calculate_fee(const fee_parameters_type& k) const
	{
		int64_t additional_interval_days = (timeout_threshold - fc::time_point::now()).to_seconds() / (60 * 60 * 24);
		return k.fee + k.daily_fee * additional_interval_days;
	}

	void htlc_refund_operation::validate() const
	{
		FC_ASSERT(htlc_id.space_id == implementation_ids);
		FC_ASSERT(htlc_id.type_id == impl_htlc_object_type);
		FC_ASSERT(amount.amount > 0);
	}

} } // graphene::chain
