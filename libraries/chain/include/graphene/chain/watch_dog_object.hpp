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
#include <graphene/chain/protocol/types.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/time.hpp>
#include <string>


namespace graphene { namespace chain {

	class watch_dog_object : public graphene::db::abstract_object<watch_dog_object>
	{
	public:
		static const uint8_t space_id = implementation_ids;
		static const uint8_t type_id = impl_watch_dog_object_type;

		enum watch_state
		{
			idle = 0,
			ready,
			question_sended,
			answer_received,
			recover_begin,
			//recover_triggered,
			recover_end,
		};

		watch_state      state = idle;
		account_id_type  watch_account;
		account_id_type  inherit_account;
		fc::sha256       answer_hash;

		fc::time_point   question_send_time;
		fc::time_point   recover_begin_time;
	};

	struct by_id;
	struct by_account_id;

	typedef multi_index_container<
		watch_dog_object,
		indexed_by<
		ordered_unique< tag<by_id>, member<object, object_id_type, &object::id> >,
		ordered_unique< tag<by_account_id>, member<watch_dog_object, account_id_type, &watch_dog_object::watch_account> >
		>
	> watch_dog_multi_index_type;

	typedef generic_index<watch_dog_object, watch_dog_multi_index_type> watch_dog_index;

}}

FC_REFLECT_ENUM(graphene::chain::watch_dog_object::watch_state, 
               (idle)(ready)(question_sended)(answer_received)
			   (recover_begin)(recover_end) )

FC_REFLECT_DERIVED(graphene::chain::watch_dog_object,
	              (graphene::db::object),
				  (state)(watch_account)(inherit_account)(answer_hash)
				  (question_send_time)(recover_begin_time))
