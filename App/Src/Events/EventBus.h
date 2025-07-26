#pragma once
#include "Utility.h"
#include "Event.h"

#define CREATE_BUS_FORWARDING_SUB(EventBus, Prop, m_EventBus) \
	listener_id Subscribe(const Prop prop, consumer<EventBus::payload_t> callable) { return m_EventBus.Subscribe(prop, callable); }		\
	template <typename C> requires in_variant<C, EventBus::payload_t>																	\
	listener_id Subscribe(const Prop prop, consumer<C> callable) { return m_EventBus.Subscribe<C>(prop, callable); }					\
	template <std::enable_if_t<variant_contains<std::monostate, EventBus::payload_t>::value, bool> = true>								\
	listener_id Subscribe(const Prop prop, callable callable) { return m_EventBus.Subscribe(prop, callable); }							\
	bool Unsubscribe(Prop prop, listener_id id) { m_EventBus.Unsubscribe(prop, id); }


template <typename... T>
class EventBus {
public:
	using payload_t = std::variant<T...>;


public:
	listener_id Subscribe(const prop_id key, consumer<payload_t> cb)
	{
		return m_Listeners[key].Subscribe(cb);
	}
	template <typename C> requires in_variant<C, payload_t>
	listener_id Subscribe(const prop_id key, consumer<C> cb)
	{
		return m_Listeners[key].Subscribe(CreateListener(cb));
	}
	listener_id Subscribe(const prop_id key, callable cb) requires in_variant<std::monostate, payload_t>
	{
		return m_Listeners[key].Subscribe(CreateListener(cb));
	}

	bool Unsubscribe(const prop_id prop, listener_id id)
	{
		return m_Listeners[prop].Unsubscribe(id);
	}
	// MAYBE: remove, as Event uses a static id
	/*u64 Unsubscribe(listener_id id)
	{
		u64 res = 0;
		for (const auto& [prop, _] : m_Listeners)
			if (Unsubscribe(prop, id))
				res++;
		return res;
	}*/

	
	void Notify(const prop_id key, const payload_t& data = {}) requires in_variant<std::monostate, payload_t>
	{
		m_Listeners[key].Notify(data);
	}
	template <typename C> requires in_variant<C, payload_t>
	void Notify(const prop_id key, const C& data)
	{
		Notify(key, payload_t{ data });
	}


	template <typename C> requires in_variant<C, payload_t>
	static consumer<payload_t> CreateListener(const consumer<C>& cb)
	{
		return [cb](payload_t p) {
			if (std::holds_alternative<C>(p))
				cb(std::get<C>(p));
		};
	}
	static consumer<payload_t> CreateListener(const callable& cb) requires in_variant<std::monostate, payload_t>
	{
		return [cb](payload_t p) {
			if (std::holds_alternative<std::monostate>(p))
				cb();
			};
	}


private:
	std::unordered_map<prop_id, Event<payload_t>> m_Listeners;
};
