/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

{% import "defs.tpl" as defs %}

#pragma once

#include <Dict.h>

namespace Nidium {
namespace Binding {

// {{ '{{{' }} Dict_{{prefix}}{{ name }}
class Dict_{{prefix}}{{ name }} : public Dict
{
public:
    Dict_{{ name }}() {
        {% for attr in members %}
            {% set type = attr.idlType.idlType %}
            {% if type == 'cstring' %}
                m_{{ attr.name }} = {{ 'NULL' if not attr.default else "strdup(" + attr.default.value + ");" }}
            {% else %}
                m_{{ attr.name }} = {{ 'NULL' if not attr.default else attr.default.value }};
            {% endif %}
        {% endfor %}
    }

    /*
        TODO dtor
    */
    bool initWithJSVal(JSContext *cx, JS::HandleValue v)
    {
        if (!v.isObject()) {
            return false;
        }
        JS::RootedObject curobj(cx, &v.toObject());
        JS::RootedValue curopt(cx);
        {% for attr in members %}
            {% set type = attr.idlType.idlType %}
            if (!JS_GetProperty(cx, curobj, "{{ attr.name }}", &curopt)) {
                return false;
            } else {
                {% if type == 'cstring' %}
                    JS::RootedString curstr(cx, JS::ToString(cx, curopt));
                    JSAutoByteString c_curstr(cx, curstr);

                    m_{{ attr.name }} = strdup(c_curstr.ptr());
                {% elif type == 'unsigned_short' %}
                    if (!JS::ToUint16(cx, curopt, &m_{{ attr.name }})) {
                        return false;
                    }
                {% endif %}
            }
        {% endfor %}

        return true;
    }
    {% for attr in members %}
        {% set type = attr.idlType.idlType %}
        const {{ attr.name }} {{ attr.name }}() const {
            return m_{{ attr.name }};
        }
    {% endfor %}
private:
    {% for attr in members %}
        {% set type = attr.idlType.idlType %}
        {{ type|ctype }} m_{{ attr.name }};
    {% endfor %}
};
// {{ '}}}' }}

} // namespace Binding
} // namespace Nidium

