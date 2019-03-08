/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
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

#include <graphene/chain/database.hpp>

#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/chain_property_object.hpp>
#include <graphene/chain/global_property_object.hpp>

#include <fc/smart_ref_impl.hpp>

namespace graphene { namespace chain {

const asset_object& database::get_core_asset() const
{
   return *_p_core_asset_obj;
}

const asset_dynamic_data_object& database::get_core_dynamic_data() const
{
   return *_p_core_dynamic_data_obj;
}

const global_property_object& database::get_global_properties()const
{
   return *_p_global_prop_obj;
}

const vm_cpu_limit_t database::get_cpu_limit() const
{
    const chain_parameters& params = get_global_properties().parameters;
    for (auto& ext : params.extensions) {
        if (ext.which() == future_extensions::tag<vm_cpu_limit_t>::value) {
            return ext.get<vm_cpu_limit_t>();
        }
    }
    // return default value
    return vm_cpu_limit_t();
}

const chain_property_object& database::get_chain_properties()const
{
   return *_p_chain_property_obj;
}

const dynamic_global_property_object& database::get_dynamic_global_properties() const
{
   return *_p_dyn_global_prop_obj;
}

const fee_schedule&  database::current_fee_schedule()const
{
   return get_global_properties().parameters.current_fees;
}

time_point_sec database::head_block_time()const
{
   return get_dynamic_global_properties().time;
}

uint32_t database::head_block_num()const
{
   return get_dynamic_global_properties().head_block_number;
}

block_id_type database::head_block_id()const
{
   return get_dynamic_global_properties().head_block_id;
}

decltype( chain_parameters::block_interval ) database::block_interval( )const
{
   return get_global_properties().parameters.block_interval;
}

const chain_id_type& database::get_chain_id( )const
{
   return get_chain_properties().chain_id;
}

const node_property_object& database::get_node_properties()const
{
   return _node_property_object;
}

node_property_object& database::node_properties()
{
   return _node_property_object;
}

uint32_t database::last_non_undoable_block_num() const
{
   return head_block_num() - _undo_db.size();
}

const account_statistics_object& database::get_account_stats_by_owner( account_id_type owner )const
{
   auto& idx = get_index_type<account_stats_index>().indices().get<by_owner>();
   auto itr = idx.find( owner );
   FC_ASSERT( itr != idx.end(), "Can not find account statistics object for owner ${a}", ("a",owner) );
   return *itr;
}

const witness_schedule_object& database::get_witness_schedule_object()const
{
   return *_p_witness_schedule_obj;
}

const htlc_object& database::get_htlc(htlc_id_type htlc_id) const
{
	auto& index = get_index_type<htlc_index>().indices().get<by_id>();
	auto iter = index.find(htlc_id);
	FC_ASSERT(iter != index.end(), "Can not find htlc object for htlc_id ${h}", ("h", htlc_id));
	return *iter;
}

vector<htlc_id_type> database::get_htlc_for_account(account_id_type account_id) const
{
	vector<htlc_id_type> vec;
	auto& index = get_index_type<htlc_index>().indices().get<by_id>();
	std::for_each(index.begin(), index.end(), [&](const htlc_object& obj)
	{
		if (obj.depositor == account_id)
			vec.push_back(obj.id);
	});
	return vec;
}

vector<htlc_id_type> database::get_expired_htlc() const
{
	vector<htlc_id_type> vec;
	auto& index = get_index_type<htlc_index>().indices().get<by_id>();
	std::for_each(index.begin(), index.end(), [&](const htlc_object& obj)
	{
		if (fc::time_point::now() > obj.expiration)
			vec.push_back(obj.id);
	});
	return vec;
}

void database::refund_expired_htlc(htlc_id_type htlc_id)
{
	auto& index = get_index_type<htlc_index>().indices().get<by_id>();
	auto iter = index.find(htlc_id);
	if (iter != index.end())
	{
		adjust_balance(iter->depositor, iter->amount);
		remove(htlc_id);
	}
}

void database::refund_all_expired_htlc()
{
	vector<htlc_id_type> vec = get_expired_htlc();
	for (const auto& htlc_id : vec)
		refund_expired_htlc(htlc_id);
}

std::vector<watch_dog_id_type> database::get_watch_dog_id_list()
{
	std::vector<watch_dog_id_type> result;
	auto& idx = get_index_type<watch_dog_index>().indices().get<by_id>();
	std::for_each(idx.begin(), idx.end(), [&](const watch_dog_object& obj)
	{
		result.push_back(obj.id);
	});
	return result;
}

std::vector<const watch_dog_object*> database::get_watch_dog_list()
{
	std::vector<const watch_dog_object*> result;
	auto& idx = get_index_type<watch_dog_index>().indices().get<by_id>();
	std::for_each(idx.begin(), idx.end(), [&](const watch_dog_object& obj)
	{
		result.push_back(&obj);
	});
	return result;
}

watch_dog_id_type database::get_watch_dog_id_by_account(account_id_type account_id)
{
	auto& idx = get_index_type<watch_dog_index>().indices().get<by_account_id>();
	auto iter = idx.find(account_id);
	FC_ASSERT(iter != idx.end(), "Can not find watch dog object for account ${a}", ("a", account_id));
	return iter->id;
}

const watch_dog_object* database::get_watch_dog_by_account(account_id_type account_id)
{
	auto& idx = get_index_type<watch_dog_index>().indices().get<by_account_id>();
	auto iter = idx.find(account_id);
	FC_ASSERT(iter != idx.end(), "Can not find watch dog object for account ${a}", ("a", account_id));
	return &*iter;
}

const watch_dog_object* database::get_watch_dog_by_id(watch_dog_id_type watch_dog_id)
{
	auto& idx = get_index_type<watch_dog_index>().indices().get<by_id>();
	auto iter = idx.find(watch_dog_id);
	FC_ASSERT(iter != idx.end(), "Can not find watch dog object ${w}", ("w", watch_dog_id));
	return &*iter;
}

bool database::check_account_have_watch_dog(account_id_type account_id)
{
	auto& idx = get_index_type<watch_dog_index>().indices().get<by_account_id>();
	auto iter = idx.find(account_id);
	if (iter != idx.end())
		return true;
	return false;
}

} }
