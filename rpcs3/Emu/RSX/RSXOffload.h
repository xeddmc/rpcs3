﻿#pragma once

#include "Utilities/types.h"
#include "Utilities/lockless.h"
#include "Utilities/Thread.h"
#include "gcm_enums.h"

#include <vector>

namespace rsx
{
	class dma_manager
	{
		enum op
		{
			raw_copy = 0,
			vector_copy = 1,
			index_emulate = 2
		};

		struct transport_packet
		{
			op type;
			std::vector<u8> opt_storage;
			void *src;
			void *dst;
			u32 length;
			u32 aux_param0;
			u32 aux_param1;

			transport_packet(void *_dst, void *_src, u32 len)
				: src(_src), dst(_dst), length(len), type(op::raw_copy)
			{}

			transport_packet(void *_dst, std::vector<u8>& _src, u32 len)
				: dst(_dst), opt_storage(std::move(_src)), length(len), type(op::vector_copy)
			{}

			transport_packet(void *_dst, rsx::primitive_type prim, u32 len)
				: dst(_dst), aux_param0(static_cast<u8>(prim)), length(len), type(op::index_emulate)
			{}
		};

		lf_queue<transport_packet> m_work_queue;
		atomic_t<u64> m_enqueued_count{ 0 };
		volatile u64 m_processed_count = 0;
		thread_state m_worker_state = thread_state::detached;

		// TODO: Improved benchmarks here; value determined by profiling on a Ryzen CPU, rounded to the nearest 512 bytes
		const u32 max_immediate_transfer_size = 3584;

	public:
		dma_manager() = default;

		// initialization
		void init();

		// General tranport
		void copy(void *dst, std::vector<u8>& src, u32 length);
		void copy(void *dst, void *src, u32 length);

		// Vertex utilities
		void emulate_as_indexed(void *dst, rsx::primitive_type primitive, u32 count);

		// Synchronization
		void sync();
		void join();
	};

	extern dma_manager g_dma_manager;
}
