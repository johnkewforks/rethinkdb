#ifndef __CONCURRENCY_WATCHABLE_HPP__
#define __CONCURRENCY_WATCHABLE_HPP__

#include "containers/intrusive_list.hpp"

/* A `watchable_t` is a variable combined with a way to get notified when the variable
changes. `watchable_t` is read-only; see `watchable_var_t` for the whole story. */

template<class value_t>
struct watchable_t {

    /* Some notes on watchers:
    1. If the variable "changes" but its new value is equal to the old value, it is
        undefined whether or not the watchers will be notified.
    2. It is legal for the `watchable_t` to call `on_watchable_changed()` when in fact
        the variable has not changed.
    If your implementation of `watcher_t` can't handle the above behaviors, then you
    probably shouldn't be using `watchable_t`.

    (In the current implementation, the watchers will not be notified if the new value
    is equal to the old value, and the watchers will never be spuriously notified.) */

    struct watcher_t : public intrusive_list_node_t<watcher_t> {
        virtual void on_watchable_changed() = 0;
    protected:
        virtual ~watcher_t() { }
    };

    void add_watcher(watcher_t *w) {
        watchers.push_back(w);
    }
    void remove_watcher(watcher_t *w) {
        watchers.remove(w);
    }

    value_t get() {
        return value;
    }

protected:
    watchable_t(const value_t &v) : value(v) { }
    void set(const value_t &v) {
        if (value != v) {
            value = v;
            for (typename intrusive_list_t<watcher_t>::iterator it = watchers.begin();
                 it != watchers.end();
                 it++) {
                (*it).on_watchable_changed();
            }
        }
    }

private:
    intrusive_list_t<watcher_t> watchers;
    value_t value;
    DISABLE_COPYING(watchable_t);
};

/* It's impossible to actually construct a `watchable_t`; instead, you must construct a
subclass of a `watchable_t` (such as `watchable_var_t`) and then you can cast down to
a `watchable_t*` to pass around. This way you can control who can write to the
watchable variable. This is similar to the relationship between `signal_t` and
`cond_t`. */

template<class value_t>
struct watchable_var_t : public watchable_t<value_t> {

    watchable_var_t() : watchable_t<value_t>(value_t()) { }
    watchable_var_t(const value_t &v) : watchable_t<value_t>(v) { }

    void set(const value_t &v) {
        watchable_t<value_t>::set(v);
    }

private:
    DISABLE_COPYING(watchable_var_t);
};

#endif /* __CONCURRENCY_WATCHABLE_HPP__ */
