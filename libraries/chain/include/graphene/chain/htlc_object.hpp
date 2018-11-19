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
#include <graphene/db/generic_index.hpp>
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/chain/protocol/asset.hpp>
#include <fc/time.hpp>
#include <vector>

namespace graphene { namespace chain {

	class htlc_object : public graphene::db::abstract_object<htlc_object> 
	{
	public:
		static const uint8_t space_id = implementation_ids;
		static const uint8_t type_id  = impl_htlc_object_type;

		account_id_type       depositor;
		account_id_type       recipient;
		asset                 amount;
		fc::time_point        expiration;
		asset                 pending_fee;

		std::string           hash_algorithm; // only "sha256", "sha1", "ripemd160" can be used
		std::string           preimage_hash;  // hex string
		uint16_t              preimage_size;
		transaction_id_type   preimage_tx_id;
	};

	struct by_id;
	struct by_depositor;

	typedef multi_index_container<
		htlc_object,
		indexed_by<
		ordered_unique< tag<by_id>, member<object, object_id_type, &object::id> >,
		ordered_non_unique< tag<by_depositor>, member<htlc_object, account_id_type, &htlc_object::depositor> >
		>
	> htlc_multi_index_type;

	typedef generic_index<htlc_object, htlc_multi_index_type> htlc_index;

} } // graphene::chain

FC_REFLECT_DERIVED(graphene::chain::htlc_object, (graphene::db::object),
                   (depositor)(recipient)(amount)(expiration)(pending_fee)(hash_algorithm)(preimage_hash)(preimage_size)(preimage_tx_id))
