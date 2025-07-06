#include "stack_offset_manager.h"
#include "koopa.h"

int StackOffsetManager::getNextId() { return ++id_counter; }

int StackOffsetManager::id_to_offset(int id) {
  if (id_to_offset_map.find(id) == id_to_offset_map.end()) {
    assert(false);
  }
  return id_to_offset_map[id] + a;
}

void StackOffsetManager::setOffset(const koopa_raw_value_t &value) {
  auto kind = value->kind.tag;
  auto ty_tag = value->ty->tag;
  switch (kind) {
  case KOOPA_RVT_LOAD:
    load_id_map[&value->kind.data.load] = getNextId();
    id_to_offset_map[load_id_map[&value->kind.data.load]] =
        current_stack_offset;
    current_stack_offset += 4;
    break;
  case KOOPA_RVT_BINARY:
    binary_id_map[&value->kind.data.binary] = getNextId();
    id_to_offset_map[binary_id_map[&value->kind.data.binary]] =
        current_stack_offset;
    current_stack_offset += 4;
    break;
  case KOOPA_RVT_ALLOC: {
    alloc_name_id_map[value->name] = getNextId();
    id_to_offset_map[alloc_name_id_map[value->name]] = current_stack_offset;
    int now_size = 4;
    if (ty_tag == KOOPA_RTT_POINTER) {
      auto tmp = value->ty->data.pointer.base;
      while (tmp->tag == KOOPA_RTT_ARRAY) {
        now_size *= tmp->data.array.len;
        tmp = tmp->data.array.base;
      }
      current_stack_offset += now_size;
    } else {
      assert(false);
    }
    break;
  }
  case KOOPA_RVT_FUNC_ARG_REF:
    func_arg_idx_id_map[value->kind.data.func_arg_ref.index] = getNextId();
    id_to_offset_map[func_arg_idx_id_map[value->kind.data.func_arg_ref.index]] =
        current_stack_offset;
    current_stack_offset += 4;
    break;
  case KOOPA_RVT_CALL:
    call_id_map[&value->kind.data.call] = getNextId();
    id_to_offset_map[call_id_map[&value->kind.data.call]] =
        current_stack_offset;
    current_stack_offset += 4;
    break;
  case KOOPA_RVT_GLOBAL_ALLOC:
    alloc_name_id_map[value->name] = getNextId();
    id_to_offset_map[alloc_name_id_map[value->name]] = current_stack_offset;
    current_stack_offset += 4;
    break;
  case KOOPA_RVT_GET_ELEM_PTR:
    get_elem_ptr_id_map[&value->kind.data.get_elem_ptr] = getNextId();
    id_to_offset_map[get_elem_ptr_id_map[&value->kind.data.get_elem_ptr]] =
        current_stack_offset;
    current_stack_offset += 4;
    break;
  case KOOPA_RVT_GET_PTR:
    // if (alloc_name_id_map.find(value->name) == alloc_name_id_map.end()) {
    //   alloc_name_id_map[value->name] = getNextId();
    // }
    get_ptr_id_map[&value->kind.data.get_ptr] = getNextId();
    id_to_offset_map[get_ptr_id_map[&value->kind.data.get_ptr]] =
        current_stack_offset;
    current_stack_offset += 4;
    break;
  case KOOPA_RVT_AGGREGATE:
  default:
    assert(false);
  }
}

void StackOffsetManager::setOffset(int idx, int offset) {
  func_arg_idx_id_map[idx] = getNextId();
  id_to_offset_map[func_arg_idx_id_map[idx]] = offset;
}

int StackOffsetManager::getOffset(const koopa_raw_binary_t &binary) {
  int id = 0;
  if (binary_id_map.find(&binary) == binary_id_map.end()) {
    assert(false);
  } else {
    id = binary_id_map[&binary];
  }
  return id_to_offset(id);
}

int StackOffsetManager::getOffset(const koopa_raw_call_t &call) {
  int id = 0;
  if (call_id_map.find(&call) == call_id_map.end()) {
    assert(false);
  } else {
    id = call_id_map[&call];
  }
  return id_to_offset(id);
}

int StackOffsetManager::getOffset(
    const koopa_raw_func_arg_ref_t &func_arg_ref) {
  int id = 0;
  if (func_arg_idx_id_map.find(func_arg_ref.index) ==
      func_arg_idx_id_map.end()) {
    assert(false);
  } else {
    id = func_arg_idx_id_map[func_arg_ref.index];
  }
  return id_to_offset(id);
}

int StackOffsetManager::getOffset(const koopa_raw_load_t &load) {
  int id = 0;
  if (load_id_map.find(&load) == load_id_map.end()) {
    assert(false);
  } else {
    id = load_id_map[&load];
  }
  return id_to_offset(id);
}

int StackOffsetManager::getOffset(const std::string &alloc_name) {
  int id = 0;
  if (alloc_name_id_map.find(alloc_name) == alloc_name_id_map.end()) {
    assert(false);
  } else {
    id = alloc_name_id_map[alloc_name];
  }
  return id_to_offset(id);
}

int StackOffsetManager::getOffset(
    const koopa_raw_get_elem_ptr_t &get_elem_ptr) {
  int id = 0;
  if (get_elem_ptr_id_map.find(&get_elem_ptr) == get_elem_ptr_id_map.end()) {
    assert(false);
  } else {
    id = get_elem_ptr_id_map[&get_elem_ptr];
  }
  return id_to_offset(id);
}

int StackOffsetManager::getOffset(const koopa_raw_get_ptr_t &get_ptr) {
  int id = 0;
  if (get_ptr_id_map.find(&get_ptr) == get_ptr_id_map.end()) {
    assert(false);
  } else {
    id = get_ptr_id_map[&get_ptr];
  }
  return id_to_offset(id);
}

int StackOffsetManager::getOffset(const koopa_raw_aggregate_t &aggregate) {
  int id = 0;
  if (aggregate_id_map.find(&aggregate) == aggregate_id_map.end()) {
    assert(false);
  } else {
    id = aggregate_id_map[&aggregate];
  }
  return id_to_offset(id);
}

int StackOffsetManager::getOffset(const koopa_raw_value_t &value) {
  switch (value->kind.tag) {
  case KOOPA_RVT_BINARY:
    return getOffset(value->kind.data.binary);
  case KOOPA_RVT_FUNC_ARG_REF:
    return getOffset(value->kind.data.func_arg_ref);
  case KOOPA_RVT_LOAD:
    return getOffset(value->kind.data.load);
  case KOOPA_RVT_CALL:
    return getOffset(value->kind.data.call);
  case KOOPA_RVT_GET_ELEM_PTR:
    return getOffset(value->kind.data.get_elem_ptr);
  case KOOPA_RVT_GET_PTR:
    return getOffset(value->kind.data.get_ptr);
  case KOOPA_RVT_AGGREGATE:
    return getOffset(value->kind.data.aggregate);
  default:
    assert(false);
  }
  return 0;
}

void StackOffsetManager::clear() {
  load_id_map.clear();
  binary_id_map.clear();
  alloc_name_id_map.clear();
  id_to_offset_map.clear();
  current_stack_offset = 0;
  id_counter = 0;
}
