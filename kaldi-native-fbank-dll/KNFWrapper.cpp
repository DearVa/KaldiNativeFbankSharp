//KNFWrapper.cpp
#include "pch.h"
#include "KNFWrapper.h"

#include <iostream>

#include "stdlib.h";
#include <cassert>


namespace knf
{
	struct KnfOnlineFbank {
		knf::OnlineFbank* impl;
	};

	FbankOptions* GetFbankOptions(float dither, bool snip_edges, float sample_rate, int32_t num_bins, float frame_shift, float frame_length, float energy_floor, bool debug_mel, const char* window_type)
	{
		FbankOptions* opts = new FbankOptions;
		opts->frame_opts.dither = 0;
		opts->frame_opts.snip_edges = snip_edges;
		opts->frame_opts.samp_freq = sample_rate;
		opts->frame_opts.window_type = window_type;
		opts->frame_opts.frame_shift_ms = frame_shift;
		opts->frame_opts.frame_length_ms = frame_length;
		opts->mel_opts.num_bins = 80;
		opts->mel_opts.debug_mel = debug_mel;
		opts->energy_floor = energy_floor;
		return opts;
	}

	KnfOnlineFbank* GetOnlineFbank(FbankOptions* opts)
	{
		KnfOnlineFbank* knfOnlineFbank = new KnfOnlineFbank;
		knfOnlineFbank->impl = new OnlineFbank(*opts);
		return knfOnlineFbank;
	}

	void AcceptWaveform(KnfOnlineFbank* knfOnlineFbank, float sample_rate, float* samples, int samples_size)
	{
		std::vector<float> waveform{ samples, samples + samples_size };
		knfOnlineFbank->impl->AcceptWaveform(sample_rate, waveform.data(), waveform.size());
	}

	void  InputFinished(KnfOnlineFbank* knfOnlineFbank) {
		knfOnlineFbank->impl->InputFinished();
	}

	int32_t  GetNumFramesReady(KnfOnlineFbank* knfOnlineFbank) {
		int32_t n = knfOnlineFbank->impl->NumFramesReady();
		return n;
	}

	void GetFbank(KnfOnlineFbank* knfOnlineFbank, int frame, FbankData* /*out*/ pData) {
		int32_t n = knfOnlineFbank->impl->NumFramesReady();
		assert(n > 0 && "Please first call AcceptWaveform()");

		int32_t feature_dim = knfOnlineFbank->impl->Dim();

		const float* f = knfOnlineFbank->impl->GetFrame(frame);
		pData->data_length = feature_dim;
		pData->data = f;
	}

	void GetFbanks(KnfOnlineFbank* knfOnlineFbank, int framesNum, FbankDatas* /*out*/ pData) {
		int32_t n = knfOnlineFbank->impl->NumFramesReady();
		assert(n > 0 && "Please first call AcceptWaveform()");
		std::vector<float> features = GetFrames(knfOnlineFbank, framesNum);
		pData->data = new float[features.size()];
		pData->data_length = features.size();
		for (int32_t i = 0; i != features.size(); ++i) {
			pData->data[i] = features[i];
		}
	}

	std::vector<float> GetFrames(KnfOnlineFbank* knfOnlineFbank, int framesNum) {
		int32_t n = framesNum;
		assert(n > 0 && "Please first call AcceptWaveform()");
		int32_t feature_dim = knfOnlineFbank->impl->Dim();
		std::vector<float> features(n * feature_dim);
		float* p = features.data();
		for (int32_t i = 0; i != n; ++i) {
			const float* f = knfOnlineFbank->impl->GetFrame(i);
			std::copy(f, f + feature_dim, p);
			p += feature_dim;
		}
		return features;
	}
}
