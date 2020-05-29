/*
 * Copyright (c) 2019, NVIDIA CORPORATION.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file reader_impl.hpp
 * @brief cuDF-IO CSV reader class implementation header
 */

#pragma once

#include "csv.h"
#include "csv_gpu.h"

#include <cudf/detail/utilities/trie.cuh>
#include <io/utilities/column_buffer.hpp>
#include <io/utilities/hostdevice_vector.hpp>

#include <cudf/io/datasource.hpp>
#include <cudf/io/readers.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace cudf {
namespace io {
namespace detail {
namespace csv {
using namespace cudf::io::csv;
using namespace cudf::io;

/**
 * @brief Implementation for CSV reader
 *
 * The CSV reader is implemented in 4 stages:
 * Stage 1: read and optionally decompress the input data in host memory
 * (may be a memory-mapped view of the data on disk)
 *
 * Stage 2: gather the offset of each data row within the csv data.
 * Since the number of rows in a given character block may depend on the
 * initial parser state (like whether the block starts in a middle of a
 * quote or not), a separate row count and output parser state is computed
 * for every possible input parser state per 16KB character block.
 * The result is then used to infer the parser state and starting row at
 * the beginning of every character block.
 * A second pass can then output the location of every row (which is needed
 * for the subsequent parallel conversion of every row from csv text
 * to cudf binary form)
 *
 * Stage 3: Optional stage to infer the data type of each CSV column.
 *
 * Stage 4: Convert every row from csv text form to cudf binary form.
 */
class reader::impl {
 public:
  /**
   * @brief Constructor from a dataset source with reader options.
   *
   * @param source Dataset source
   * @param filepath Filepath if reading dataset from a file
   * @param options Settings for controlling reading behavior
   * @param mr Device memory resource to use for device memory allocation
   */
  explicit impl(std::unique_ptr<datasource> source,
                std::string filepath,
                reader_options const &options,
                rmm::mr::device_memory_resource *mr);

  /**
   * @brief Read an entire set or a subset of data and returns a set of columns.
   *
   * @param range_offset Number of bytes offset from the start
   * @param range_size Bytes to read; use `0` for all remaining data
   * @param skip_rows Number of rows to skip from the start
   * @param skip_rows_end Number of rows to skip from the end
   * @param num_rows Number of rows to read
   * @param metadata Optional location to return table metadata
   * @param stream CUDA stream used for device memory operations and kernel launches.
   *
   * @return The set of columns along with metadata
   */
  table_with_metadata read(size_t range_offset,
                           size_t range_size,
                           int skip_rows,
                           int skip_end_rows,
                           int num_rows,
                           cudaStream_t stream);

 private:
  /**
   * @brief Finds row positions within the specified input data.
   *
   * This function scans the input data to record the row offsets (relative to
   * the start of the input data).
   * A row is actually the data/offset between two termination symbols.
   *
   * @param h_data Uncompressed input data in host memory
   * @param h_size Number of bytes of uncompressed input data
   * @param range_begin Only include rows starting after this position
   * @param range_end Only include rows starting before this position
   * @param skip_rows Number of rows to skip from the start
   * @param num_rows Number of rows to read; -1: all remaining data
   * @param load_whole_file Hint that the entire data will be needed on gpu
   * @param stream CUDA stream used for device memory operations and kernel launches.
   */
  void gather_row_offsets(const char *h_data,
                          size_t h_size,
                          size_t range_begin,
                          size_t range_end,
                          size_t skip_rows,
                          int64_t num_rows,
                          bool load_whole_file,
                          cudaStream_t stream);

  /**
   * @brief Find the start position of the first data row
   *
   * @param h_data Uncompressed input data in host memory
   * @param h_size Number of bytes of uncompressed input data
   *
   * @return Byte position of the first row
   */
  size_t find_first_row_start(const char *h_data, size_t h_size);

  /**
   * @brief Returns a detected or parsed list of column dtypes.
   *
   * @param stream CUDA stream used for device memory operations and kernel launches.
   *
   * @return `std::vector<data_type>` List of column types
   */
  std::vector<data_type> gather_column_types(cudaStream_t stream);

  /**
   * @brief Converts the row-column data and outputs to columns.
   *
   * @param column_types Column types
   * @param out_buffers Output columns' device buffers
   * @param stream CUDA stream used for device memory operations and kernel launches.
   */
  void decode_data(std::vector<data_type> const &column_types,
                   std::vector<column_buffer> &out_buffers,
                   cudaStream_t stream);

 private:
  rmm::mr::device_memory_resource *mr_ = nullptr;
  std::unique_ptr<datasource> source_;
  std::string filepath_;
  std::string compression_type_;
  const reader_options args_;

  rmm::device_vector<char> data_;
  rmm::device_vector<uint64_t> row_offsets;
  size_t num_records  = 0;  // Number of rows with actual data
  int num_active_cols = 0;  // Number of columns to read
  int num_actual_cols = 0;  // Number of columns in the dataset

  // Parsing options
  ParseOptions opts{};
  thrust::host_vector<column_parse::flags> h_column_flags;
  rmm::device_vector<column_parse::flags> d_column_flags;
  rmm::device_vector<SerialTrieNode> d_trueTrie;
  rmm::device_vector<SerialTrieNode> d_falseTrie;
  rmm::device_vector<SerialTrieNode> d_naTrie;

  // Intermediate data
  std::vector<std::string> col_names;
  std::vector<char> header;
};

}  // namespace csv
}  // namespace detail
}  // namespace io
}  // namespace cudf
