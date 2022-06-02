# =============================================================================
# Copyright (c) 2022, NVIDIA CORPORATION.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
# in compliance with the License. You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software distributed under the License
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
# or implied. See the License for the specific language governing permissions and limitations under
# the License.
# =============================================================================

set(cython_sources edit_distance.pyx generate_ngrams.pyx ngrams_tokenize.pyx normalize.pyx
                   replace.pyx stemmer.pyx subword_tokenize.pyx tokenize.pyx
)
set(linked_libraries cudf::cudf)
rapids_cython_create_modules(
  CXX
  SOURCE_FILES "${cython_sources}"
  LINKED_LIBRARIES "${linked_libraries}" MODULE_PREFIX nvtext_
)

foreach(cython_module IN LISTS RAPIDS_CYTHON_CREATED_TARGETS)
  set_target_properties(${cython_module} PROPERTIES INSTALL_RPATH "\$ORIGIN;\$ORIGIN/../cpp")
endforeach()