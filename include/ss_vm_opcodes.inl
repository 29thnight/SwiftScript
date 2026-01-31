#pragma once
#include "ss_opcodes.hpp"
#include "ss_vm.hpp"

namespace swiftscript {

    // Note: primary template `OpCodeHandler` and `make_handler_table` are
    // declared in `ss_vm.hpp`. This file provides explicit specializations
    // only. Do not redeclare the primary template or pull the namespace into
    // global scope.
    // Build the handler table by taking pointers to each OpCodeHandler::execute

    template<>
    struct OpCodeHandler<OpCode::OP_CONSTANT> {
        static void execute(VM& vm) {
            vm.push(vm.read_constant());
        }
    };

    template<>
    struct OpCodeHandler<OpCode::OP_NIL> {
        static void execute(VM& vm) {
            vm.push(Value::null());
        }
    };

    template<>
    struct OpCodeHandler<OpCode::OP_TRUE> {
        static void execute(VM& vm) {
            vm.push(Value::from_bool(true));
        }
    };

    template<>
    struct OpCodeHandler<OpCode::OP_FALSE> {
        static void execute(VM& vm) {
            vm.push(Value::from_bool(false));
        }
    };

    template<>
    struct OpCodeHandler<OpCode::OP_POP> {
        static void execute(VM& vm) {
            vm.pop();
        }
    };

    template<>
    struct OpCodeHandler<OpCode::OP_DUP> {
        static void execute(VM& vm) {
            vm.push(vm.peek(0));
        }
    };

    // ============================================================================
    // Arithmetic Operations
    // ============================================================================

    template<>
    struct OpCodeHandler<OpCode::OP_ADD> {
        static void execute(VM& vm) {
            Value b = vm.pop();
            Value a = vm.pop();
            if (a.is_int() && b.is_int()) {
                vm.push(Value::from_int(a.as_int() + b.as_int()));
            }
            else {
                auto fa = a.try_as<Float>();
                auto fb = b.try_as<Float>();
                if (!fa || !fb) {
                    if (auto overloaded = vm.call_operator_overload(a, b, "+")) {
                        vm.push(*overloaded);
                        return;
                    }
                    throw std::runtime_error("Operands must be numbers for addition.");
                }
                vm.push(Value::from_float(*fa + *fb));
            }
        }
    };

    template<>
    struct OpCodeHandler<OpCode::OP_SUBTRACT> {
        static void execute(VM& vm) {
            Value b = vm.pop();
            Value a = vm.pop();
            auto fa = a.try_as<Float>();
            auto fb = b.try_as<Float>();
            if (!fa || !fb) {
                if (auto overloaded = vm.call_operator_overload(a, b, "-")) {
                    vm.push(*overloaded);
                    return;
                }
                throw std::runtime_error("Operands must be numbers for subtraction.");
            }
            if (a.is_int() && b.is_int()) {
                vm.push(Value::from_int(a.as_int() - b.as_int()));
            }
            else {
                vm.push(Value::from_float(*fa - *fb));
            }
        }
    };

    template<>
    struct OpCodeHandler<OpCode::OP_MULTIPLY> {
        static void execute(VM& vm) {
            Value b = vm.pop();
            Value a = vm.pop();
            auto fa = a.try_as<Float>();
            auto fb = b.try_as<Float>();
            if (!fa || !fb) {
                if (auto overloaded = vm.call_operator_overload(a, b, "*")) {
                    vm.push(*overloaded);
                    return;
                }
                throw std::runtime_error("Operands must be numbers for multiplication.");
            }
            if (a.is_int() && b.is_int()) {
                vm.push(Value::from_int(a.as_int() * b.as_int()));
            }
            else {
                vm.push(Value::from_float(*fa * *fb));
            }
        }
    };

    template<>
    struct OpCodeHandler<OpCode::OP_DIVIDE> {
        static void execute(VM& vm) {
            Value b = vm.pop();
            Value a = vm.pop();
            auto fa = a.try_as<Float>();
            auto fb = b.try_as<Float>();
            if (!fa || !fb) {
                if (auto overloaded = vm.call_operator_overload(a, b, "/")) {
                    vm.push(*overloaded);
                    return;
                }
                throw std::runtime_error("Operands must be numbers for division.");
            }
            vm.push(Value::from_float(*fa / *fb));
        }
    };

    // ============================================================================
    // Control Flow
    // ============================================================================

    template<>
    struct OpCodeHandler<OpCode::OP_JUMP> {
        static void execute(VM& vm) {
            uint16_t offset = vm.read_short();
            vm.ip_ += offset;
        }
    };

    template<>
    struct OpCodeHandler<OpCode::OP_JUMP_IF_FALSE> {
        static void execute(VM& vm) {
            uint16_t offset = vm.read_short();
            if (!vm.is_truthy(vm.peek(0))) {
                vm.ip_ += offset;
            }
        }
    };

    template<>
    struct OpCodeHandler<OpCode::OP_LOOP> {
        static void execute(VM& vm) {
            uint16_t offset = vm.read_short();
            vm.ip_ -= offset;
        }
    };

    // ============================================================================
    // Variables
    // ============================================================================

    template<>
    struct OpCodeHandler<OpCode::OP_GET_GLOBAL> {
        static void execute(VM& vm) {
            const std::string& name = vm.read_string();
            vm.push(vm.get_global(name));
        }
    };

    template<>
    struct OpCodeHandler<OpCode::OP_SET_GLOBAL> {
        static void execute(VM& vm) {
            const std::string& name = vm.read_string();
            vm.set_global(name, vm.peek(0));
        }
    };

    template<>
    struct OpCodeHandler<OpCode::OP_GET_LOCAL> {
        static void execute(VM& vm) {
            uint16_t slot = vm.read_short();
            size_t base = vm.current_stack_base();
            if (base + slot >= vm.stack_.size()) {
                throw std::runtime_error("Local slot out of range.");
            }
            vm.push(vm.stack_[base + slot]);
        }
    };

    template<>
    struct OpCodeHandler<OpCode::OP_SET_LOCAL> {
        static void execute(VM& vm) {
            uint16_t slot = vm.read_short();
            size_t base = vm.current_stack_base();
            if (base + slot >= vm.stack_.size()) {
                throw std::runtime_error("Local slot out of range.");
            }
            vm.stack_[base + slot] = vm.peek(0);
        }
    };

    // ============================================================================
    // Property Access Handlers (중간 복잡도)
    // ============================================================================

    template<>
    struct OpCodeHandler<OpCode::OP_GET_PROPERTY> {
        static void execute(VM& vm) {
            const std::string& name = vm.read_string();
            Value obj = vm.pop();
            bool handled_computed = false;

            // Check for computed property first (for instances)
            if (obj.is_object() && obj.as_object()->type == ObjectType::Instance) {
                auto* inst = static_cast<InstanceObject*>(obj.as_object());
                if (inst->klass) {
                    for (const auto& comp_prop : inst->klass->computed_properties) {
                        if (comp_prop.name == name) {
                            // Found computed property - call getter
                            Value getter = comp_prop.getter;

                            if (!getter.is_object()) {
                                throw std::runtime_error("Computed property getter is not a function.");
                            }

                            Object* obj_callee = getter.as_object();
                            FunctionObject* func = nullptr;
                            ClosureObject* closure = nullptr;

                            if (obj_callee->type == ObjectType::Closure) {
                                closure = static_cast<ClosureObject*>(obj_callee);
                                func = closure->function;
                            }
                            else if (obj_callee->type == ObjectType::Function) {
                                func = static_cast<FunctionObject*>(obj_callee);
                            }
                            else {
                                throw std::runtime_error("Computed property getter must be a function.");
                            }

                            if (!func || !func->chunk) {
                                throw std::runtime_error("Getter function has no body.");
                            }
                            if (func->params.size() != 1) {
                                throw std::runtime_error("Getter should have exactly 1 parameter (self).");
                            }

                            // Push callee and self argument
                            vm.push(getter);
                            vm.push(obj);

                            // Setup call frame
                            size_t callee_index = vm.stack_.size() - 2;
                            vm.call_frames_.emplace_back(callee_index + 1, vm.ip_, vm.chunk_, func->name, closure, false);
                            vm.chunk_ = func->chunk.get();
                            vm.ip_ = 0;
                            handled_computed = true;
                            break;
                        }
                    }
                }
            }

            if (!handled_computed) {
                // Check EnumCase and StructInstance similarly...
                vm.push(vm.get_property(obj, name));
            }
        }
    };

    template<>
    struct OpCodeHandler<OpCode::OP_SET_PROPERTY> {
        static void execute(VM& vm) {
            const std::string& name = vm.read_string();
            Value value = vm.pop();
            Value obj_val = vm.peek(0);

            if (!obj_val.is_object()) {
                throw std::runtime_error("Property set on non-object.");
            }
            Object* obj = obj_val.as_object();
            if (!obj) {
                throw std::runtime_error("Null object in property set.");
            }

            if (obj->type == ObjectType::Instance) {
                auto* inst = static_cast<InstanceObject*>(obj);

                // Check if it's a computed property
                if (inst->klass) {
                    for (const auto& comp_prop : inst->klass->computed_properties) {
                        if (comp_prop.name == name) {
                            if (comp_prop.setter.is_null()) {
                                throw std::runtime_error("Cannot set read-only computed property: " + name);
                            }

                            Value setter = comp_prop.setter;
                            vm.pop();

                            vm.push(setter);
                            vm.push(obj_val);
                            vm.push(value);

                            size_t arg_count = 2;
                            size_t callee_index = vm.stack_.size() - arg_count - 1;
                            Value callee = vm.stack_[callee_index];

                            if (!callee.is_object()) {
                                throw std::runtime_error("Computed property setter is not a function.");
                            }

                            Object* obj_callee = callee.as_object();
                            FunctionObject* func = nullptr;
                            ClosureObject* closure = nullptr;

                            if (obj_callee->type == ObjectType::Closure) {
                                closure = static_cast<ClosureObject*>(obj_callee);
                                func = closure->function;
                            }
                            else if (obj_callee->type == ObjectType::Function) {
                                func = static_cast<FunctionObject*>(obj_callee);
                            }
                            else {
                                throw std::runtime_error("Computed property setter must be a function.");
                            }

                            if (arg_count != func->params.size()) {
                                throw std::runtime_error("Incorrect argument count for computed property setter.");
                            }
                            if (!func->chunk) {
                                throw std::runtime_error("Setter function has no body.");
                            }

                            vm.call_frames_.emplace_back(callee_index + 1, vm.ip_, vm.chunk_, func->name, closure, false);
                            vm.chunk_ = func->chunk.get();
                            vm.ip_ = 0;
                            return;
                        }
                    }
                }

                // Regular stored property with observers
                Value will_set_observer = Value::null();
                Value did_set_observer = Value::null();
                Value old_value = Value::null();

                if (inst->klass) {
                    for (const auto& prop_info : inst->klass->properties) {
                        if (prop_info.name == name) {
                            will_set_observer = prop_info.will_set_observer;
                            did_set_observer = prop_info.did_set_observer;
                            auto it = inst->fields.find(name);
                            if (it != inst->fields.end()) {
                                old_value = it->second;
                            }
                            break;
                        }
                    }
                }

                vm.pop();

                if (!will_set_observer.is_null()) {
                    vm.call_property_observer(will_set_observer, obj_val, value);
                }

                inst->fields[name] = value;

                if (!did_set_observer.is_null()) {
                    vm.call_property_observer(did_set_observer, obj_val, old_value);
                }

                vm.push(value);
            }
            else if (obj->type == ObjectType::Map) {
                vm.pop();
                auto* dict = static_cast<MapObject*>(obj);
                dict->entries[name] = value;
                vm.push(value);
            }
            else {
                throw std::runtime_error("Property set only supported on instances or maps.");
            }
        }
    };

    // ============================================================================
    // Closure Handler
    // ============================================================================

    template<>
    struct OpCodeHandler<OpCode::OP_CLOSURE> {
        static void execute(VM& vm) {
            uint16_t index = vm.read_short();
            if (index >= vm.chunk_->functions.size()) {
                throw std::runtime_error("Function index out of range.");
            }

            const auto& proto = vm.chunk_->functions[index];
            std::vector<Value> defaults;
            std::vector<bool> has_defaults;
            vm.build_param_defaults(proto, defaults, has_defaults);

            auto* func = vm.allocate_object<FunctionObject>(
                proto.name,
                proto.params,
                proto.param_labels,
                std::move(defaults),
                std::move(has_defaults),
                proto.chunk,
                proto.is_initializer,
                proto.is_override);

            auto* closure = vm.allocate_object<ClosureObject>(func);
            closure->upvalues.resize(proto.upvalues.size(), nullptr);

            ClosureObject* enclosing_closure = vm.call_frames_.empty() ? nullptr : vm.call_frames_.back().closure;
            size_t base = vm.current_stack_base();

            for (size_t i = 0; i < proto.upvalues.size(); ++i) {
                const auto& uv = proto.upvalues[i];
                if (uv.is_local) {
                    if (base + uv.index >= vm.stack_.size()) {
                        throw std::runtime_error("Upvalue local slot out of range.");
                    }
                    closure->upvalues[i] = vm.capture_upvalue(&vm.stack_[base + uv.index]);
                }
                else {
                    if (!enclosing_closure) {
                        throw std::runtime_error("Upvalue refers to enclosing closure, but none is active.");
                    }
                    if (uv.index >= enclosing_closure->upvalues.size()) {
                        throw std::runtime_error("Upvalue index out of range.");
                    }
                    closure->upvalues[i] = enclosing_closure->upvalues[uv.index];
                }
            }

            vm.push(Value::from_object(closure));
        }
    };

    // ============================================================================
    // Upvalue Handlers
    // ============================================================================

    template<>
    struct OpCodeHandler<OpCode::OP_GET_UPVALUE> {
        static void execute(VM& vm) {
            uint16_t slot = vm.read_short();
            if (vm.call_frames_.empty() || !vm.call_frames_.back().closure) {
                throw std::runtime_error("No closure active for upvalue read.");
            }
            auto* closure = vm.call_frames_.back().closure;
            if (slot >= closure->upvalues.size()) {
                throw std::runtime_error("Upvalue index out of range.");
            }
            vm.push(*closure->upvalues[slot]->location);
        }
    };

    template<>
    struct OpCodeHandler<OpCode::OP_SET_UPVALUE> {
        static void execute(VM& vm) {
            uint16_t slot = vm.read_short();
            if (vm.call_frames_.empty() || !vm.call_frames_.back().closure) {
                throw std::runtime_error("No closure active for upvalue write.");
            }
            auto* closure = vm.call_frames_.back().closure;
            if (slot >= closure->upvalues.size()) {
                throw std::runtime_error("Upvalue index out of range.");
            }
            *closure->upvalues[slot]->location = vm.peek(0);
        }
    };

    template<>
    struct OpCodeHandler<OpCode::OP_CLOSE_UPVALUE> {
        static void execute(VM& vm) {
            if (vm.stack_.empty()) {
                throw std::runtime_error("Stack underflow on close upvalue.");
            }
            vm.close_upvalues(&vm.stack_.back());
            vm.pop();
        }
    };

    // specialization. This must be constexpr so it can be used to initialize
    // the global table in a translation unit.
    constexpr std::array<OpHandlerFunc, 256> make_handler_table() {
        std::array<OpHandlerFunc, 256> tbl{};
        for (auto& e : tbl) e = nullptr;

        // Register implemented handlers
        tbl[static_cast<uint8_t>(OpCode::OP_CONSTANT)] = &OpCodeHandler<OpCode::OP_CONSTANT>::execute;
        tbl[static_cast<uint8_t>(OpCode::OP_NIL)] = &OpCodeHandler<OpCode::OP_NIL>::execute;
        tbl[static_cast<uint8_t>(OpCode::OP_TRUE)] = &OpCodeHandler<OpCode::OP_TRUE>::execute;
        tbl[static_cast<uint8_t>(OpCode::OP_FALSE)] = &OpCodeHandler<OpCode::OP_FALSE>::execute;
        tbl[static_cast<uint8_t>(OpCode::OP_POP)] = &OpCodeHandler<OpCode::OP_POP>::execute;
        tbl[static_cast<uint8_t>(OpCode::OP_DUP)] = &OpCodeHandler<OpCode::OP_DUP>::execute;

        tbl[static_cast<uint8_t>(OpCode::OP_ADD)] = &OpCodeHandler<OpCode::OP_ADD>::execute;
        tbl[static_cast<uint8_t>(OpCode::OP_SUBTRACT)] = &OpCodeHandler<OpCode::OP_SUBTRACT>::execute;
        tbl[static_cast<uint8_t>(OpCode::OP_MULTIPLY)] = &OpCodeHandler<OpCode::OP_MULTIPLY>::execute;
        tbl[static_cast<uint8_t>(OpCode::OP_DIVIDE)] = &OpCodeHandler<OpCode::OP_DIVIDE>::execute;

        tbl[static_cast<uint8_t>(OpCode::OP_JUMP)] = &OpCodeHandler<OpCode::OP_JUMP>::execute;
        tbl[static_cast<uint8_t>(OpCode::OP_JUMP_IF_FALSE)] = &OpCodeHandler<OpCode::OP_JUMP_IF_FALSE>::execute;
        tbl[static_cast<uint8_t>(OpCode::OP_LOOP)] = &OpCodeHandler<OpCode::OP_LOOP>::execute;

        tbl[static_cast<uint8_t>(OpCode::OP_GET_GLOBAL)] = &OpCodeHandler<OpCode::OP_GET_GLOBAL>::execute;
        tbl[static_cast<uint8_t>(OpCode::OP_SET_GLOBAL)] = &OpCodeHandler<OpCode::OP_SET_GLOBAL>::execute;
        tbl[static_cast<uint8_t>(OpCode::OP_GET_LOCAL)] = &OpCodeHandler<OpCode::OP_GET_LOCAL>::execute;
        tbl[static_cast<uint8_t>(OpCode::OP_SET_LOCAL)] = &OpCodeHandler<OpCode::OP_SET_LOCAL>::execute;

        tbl[static_cast<uint8_t>(OpCode::OP_GET_PROPERTY)] = &OpCodeHandler<OpCode::OP_GET_PROPERTY>::execute;
        tbl[static_cast<uint8_t>(OpCode::OP_SET_PROPERTY)] = &OpCodeHandler<OpCode::OP_SET_PROPERTY>::execute;

        tbl[static_cast<uint8_t>(OpCode::OP_CLOSURE)] = &OpCodeHandler<OpCode::OP_CLOSURE>::execute;

        tbl[static_cast<uint8_t>(OpCode::OP_GET_UPVALUE)] = &OpCodeHandler<OpCode::OP_GET_UPVALUE>::execute;
        tbl[static_cast<uint8_t>(OpCode::OP_SET_UPVALUE)] = &OpCodeHandler<OpCode::OP_SET_UPVALUE>::execute;
        tbl[static_cast<uint8_t>(OpCode::OP_CLOSE_UPVALUE)] = &OpCodeHandler<OpCode::OP_CLOSE_UPVALUE>::execute;

        return tbl;
    }
}