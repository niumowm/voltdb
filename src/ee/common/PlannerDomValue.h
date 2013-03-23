/* This file is part of VoltDB.
 * Copyright (C) 2008-2013 VoltDB Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with VoltDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLANNERDOMVALUE_H_
#define PLANNERDOMVALUE_H_

#include "common/SerializableEEException.h"

#include "jsoncpp/jsoncpp.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>

namespace voltdb {

    /**
     * Represents a JSON value in a parser-library-neutral kind of way. It throws
     * VoltDB-style exceptions when things are amiss and should be otherwise pretty
     * simple to figure out how to use. See plannodes or expressions for examples.
     *
     * It might require some fudging to move from rapidjson to jsoncpp or something,
     * but WAY less fudging than it would take to edit every bit of code that uses
     * this shim.
     */
    class PlannerDomValue {
        friend class PlannerDomRoot;
    public:

        int32_t asInt() const {
            if (m_value.isNull() || (m_value.isInt() == false)) {
                throw SerializableEEException(VOLT_EE_EXCEPTION_TYPE_EEEXCEPTION,
                                              "PlannerDomValue: int value is null or not an integer");
            }
            return m_value.asInt();
        }

        int64_t asInt64() const {
            if (m_value.isNull()) {
                throw SerializableEEException(VOLT_EE_EXCEPTION_TYPE_EEEXCEPTION,
                                              "PlannerDomValue: int64 value is null");
            }
            else if (m_value.isInt64()) {
                return m_value.asInt64();
            }
            else if (m_value.isInt()) {
                return m_value.asInt();
            }
            throw SerializableEEException(VOLT_EE_EXCEPTION_TYPE_EEEXCEPTION,
                                          "PlannerDomValue: int64 value is non-integral");
        }

        double asDouble() const {
            if (m_value.isNull()) {
                throw SerializableEEException(VOLT_EE_EXCEPTION_TYPE_EEEXCEPTION,
                                              "PlannerDomValue: double value is null");
            }
            else if (m_value.isDouble()) {
                return m_value.asDouble();
            }
            else if (m_value.isInt()) {
                return m_value.asInt();
            }
            else if (m_value.isInt64()) {
                return (double) m_value.asInt64();
            }
            throw SerializableEEException(VOLT_EE_EXCEPTION_TYPE_EEEXCEPTION,
                                          "PlannerDomValue: double value is not a number");
        }

        bool asBool() const {
            if (m_value.isNull() || (m_value.isBool() == false)) {
                char msg[1024];
                snprintf(msg, 1024, "PlannerDomValue: value is null or not a bool");
                throw SerializableEEException(VOLT_EE_EXCEPTION_TYPE_EEEXCEPTION, msg);
            }
            return m_value.asBool();
        }

        std::string asStr() const {
            if (m_value.isNull() || (m_value.isString() == false)) {
                char msg[1024];
                snprintf(msg, 1024, "PlannerDomValue: value is null or not a string");
                throw SerializableEEException(VOLT_EE_EXCEPTION_TYPE_EEEXCEPTION, msg);
            }
            return m_value.asString();

        }

        bool hasKey(const char *key) const {
            return m_value.isMember(key);
        }

        bool hasNonNullKey(const char *key) const {
            if (!hasKey(key)) {
                return false;
            }
            Json::Value &value = m_value[key];
            return !value.isNull();
        }

        PlannerDomValue valueForKey(const char *key) const {
            Json::Value &value = m_value[key];
            if (value.isNull()) {
                char msg[1024];
                snprintf(msg, 1024, "PlannerDomValue: %s key is null or missing", key);
                throw SerializableEEException(VOLT_EE_EXCEPTION_TYPE_EEEXCEPTION, msg);
            }
            return PlannerDomValue(value);
        }

        int arrayLen() const {
            if (m_value.isArray() == false) {
                char msg[1024];
                snprintf(msg, 1024, "PlannerDomValue: value is not an array");
                throw SerializableEEException(VOLT_EE_EXCEPTION_TYPE_EEEXCEPTION, msg);
            }
            return m_value.size();
        }

        PlannerDomValue valueAtIndex(int index) const {
            if (m_value.isArray() == false) {
                char msg[1024];
                snprintf(msg, 1024, "PlannerDomValue: value is not an array");
                throw SerializableEEException(VOLT_EE_EXCEPTION_TYPE_EEEXCEPTION, msg);
            }
            return m_value[index];
        }

    private:
        PlannerDomValue(Json::Value &value) : m_value(value) {}

        Json::Value &m_value;
    };

    /**
     * Class that parses the JSON document and provides the root.
     * Also owns the memory, as it's sole member var is not a reference, but a value.
     * This means if you're still using the DOM when this object gets popped off the
     * stack, bad things might happen. Best to use the DOM and be done with it.
     */
    class PlannerDomRoot {
    public:
        PlannerDomRoot(const char *jsonStr) {
            Json::Reader reader;
            bool parsingSuccessful = reader.parse(jsonStr, m_document);
            if (!parsingSuccessful) {
                throw SerializableEEException(VOLT_EE_EXCEPTION_TYPE_EEEXCEPTION,
                                          "PlannerDomValue: can't parse JSON");
            }
        }

        bool isNull() {
            return m_document.isNull();
        }

        PlannerDomValue rootObject() {
            return PlannerDomValue(m_document);
        }

    private:
        Json::Value m_document;
    };
}

#endif // PLANNERDOMVALUE_H_
