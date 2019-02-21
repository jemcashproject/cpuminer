//
//
//#pragma once 
#include "mtp.h"
#include "sha3/sph_blake.h"
#ifdef _MSC_VER
#include <windows.h>
#include <winbase.h> /* For SecureZeroMemory */
#endif

#include <ios>
#include <stdio.h>
#include <iostream>
#if defined __STDC_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
#endif


#define memcost 4*1024*1024
static const unsigned int d_mtp = 1;
static const uint8_t L = 64;
static const unsigned int memory_cost = memcost;


static const unsigned char blake2b_sigma[12][16] =
{
	{ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 } ,
	{ 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 } ,
	{ 11,  8, 12,  0,  5,  2, 15, 13, 10, 14,  3,  6,  7,  1,  9,  4 } ,
	{ 7,  9,  3,  1, 13, 12, 11, 14,  2,  6,  5, 10,  4,  0, 15,  8 } ,
	{ 9,  0,  5,  7,  2,  4, 10, 15, 14,  1, 11, 12,  6,  8,  3, 13 } ,
	{ 2, 12,  6, 10,  0, 11,  8,  3,  4, 13,  7,  5, 15, 14,  1,  9 } ,
	{ 12,  5,  1, 15, 14, 13,  4, 10,  0,  7,  6,  3,  9,  2,  8, 11 } ,
	{ 13, 11,  7, 14, 12,  1,  3,  9,  5,  0, 15,  4,  8,  6,  2, 10 } ,
	{ 6, 15, 14,  9, 11,  3,  0,  8, 12,  2, 13,  7,  1,  4, 10,  5 } ,
	{ 10,  2,  8,  4,  7,  6,  1,  5, 15, 11,  9, 14,  3, 12, 13 , 0 } ,
	{ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 } ,
	{ 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 }
};

static  const uint64_t blakeIV_[8] = {
	0x6a09e667f3bcc908UL,
	0xbb67ae8584caa73bUL,
	0x3c6ef372fe94f82bUL,
	0xa54ff53a5f1d36f1UL,
	0x510e527fade682d1UL,
	0x9b05688c2b3e6c1fUL,
	0x1f83d9abfb41bd6bUL,
	0x5be0cd19137e2179UL
};
static inline uint64_t ROTR64X(const uint64_t x2, const int y) {
	return (x2 >> y) | (x2 << (64 - y));
}

static inline uint64_t eorswap64(uint64_t u, uint64_t v)
{
	return ROTR64X(u^v, 32);
}

static inline int blake2b_compress2_256(uint64_t *hash, const uint64_t *hzcash, const  uint64_t *block, const uint32_t len)
{
	uint64_t m[16];
	uint64_t v[16];

#pragma unroll
	for (int i = 0; i < 16; ++i)
		m[i] = block[i];
#pragma unroll
	for (int i = 0; i < 8; ++i)
		v[i] = hzcash[i];

	v[8] = blakeIV_[0];
	v[9] = blakeIV_[1];
	v[10] = blakeIV_[2];
	v[11] = blakeIV_[3];
	v[12] = blakeIV_[4];
	v[12] ^= len;
	v[13] = blakeIV_[5];
	v[14] = ~blakeIV_[6];
	v[15] = blakeIV_[7];


#define G(r,i,a,b,c,d) \
   { \
     v[a] +=   v[b] + (m[blake2b_sigma[r][2*i+0]]); \
     v[d] = eorswap64(v[d] , v[a]); \
     v[c] += v[d]; \
     v[b] = ROTR64X(v[b] ^ v[c], 24); \
     v[a] += v[b] + (m[blake2b_sigma[r][2*i+1]]); \
     v[d] = ROTR64X(v[d] ^ v[a], 16); \
     v[c] += v[d]; \
     v[b] = ROTR64X(v[b] ^ v[c], 63); \
  } 

#define ROUND(r)  \
  { \
    G(r,0, 0,4,8,12); \
    G(r,1, 1,5,9,13); \
    G(r,2, 2,6,10,14); \
    G(r,3, 3,7,11,15); \
    G(r,4, 0,5,10,15); \
    G(r,5, 1,6,11,12); \
    G(r,6, 2,7,8,13); \
    G(r,7, 3,4,9,14); \
  } 

	ROUND(0);
	ROUND(1);
	ROUND(2);
	ROUND(3);
	ROUND(4);
	ROUND(5);
	ROUND(6);
	ROUND(7);
	ROUND(8);
	ROUND(9);
	ROUND(10);
	ROUND(11);
#pragma unroll
	for (int i = 0; i < 4; ++i)
		hash[i] = hzcash[i] ^ v[i] ^ v[i + 8];

#undef G
#undef ROUND
	return 0;
}


static inline int blake2b_compress2b_new(uint64_t *hzcash, const  uint64_t *  block, const  uint64_t *  block0, const uint32_t len, int last)
{

	uint64_t m[16];
	uint64_t v[16];

#pragma unroll
	for (int i = 0; i < 4; ++i)
		m[i] = block0[i];

#pragma unroll
	for (int i = 4; i < 16; ++i)
		m[i] = block[i - 4];

#pragma unroll
	for (int i = 0; i < 8; ++i)
		v[i] = hzcash[i];


	v[8] = blakeIV_[0];
	v[9] = blakeIV_[1];
	v[10] = blakeIV_[2];
	v[11] = blakeIV_[3];
	v[12] = blakeIV_[4];
	v[12] ^= len;
	v[13] = blakeIV_[5];
	v[14] = last ? ~blakeIV_[6] : blakeIV_[6];
	v[15] = blakeIV_[7];


#define G(r,i,a,b,c,d) \
   { \
     v[a] +=   v[b] + (m[blake2b_sigma[r][2*i+0]]); \
     v[d] = eorswap64(v[d] , v[a]); \
     v[c] += v[d]; \
     v[b] = ROTR64X(v[b] ^ v[c], 24); \
     v[a] += v[b] + (m[blake2b_sigma[r][2*i+1]]); \
     v[d] = ROTR64X(v[d] ^ v[a], 16); \
     v[c] += v[d]; \
     v[b] = ROTR64X(v[b] ^ v[c], 63); \
  } 
#define ROUND(r)  \
  { \
    G(r,0, 0,4,8,12); \
    G(r,1, 1,5,9,13); \
    G(r,2, 2,6,10,14); \
    G(r,3, 3,7,11,15); \
    G(r,4, 0,5,10,15); \
    G(r,5, 1,6,11,12); \
    G(r,6, 2,7,8,13); \
    G(r,7, 3,4,9,14); \
  } 

	ROUND(0);
	ROUND(1);
	ROUND(2);
	ROUND(3);
	ROUND(4);
	ROUND(5);
	ROUND(6);
	ROUND(7);
	ROUND(8);
	ROUND(9);
	ROUND(10);
	ROUND(11);

	for (int i = 0; i < 8; ++i)
		hzcash[i] ^= v[i] ^ v[i + 8];

#undef G
#undef ROUND
	return 0;
}



uint32_t index_beta(const argon2_instance_t *instance,
	const argon2_position_t *position, uint32_t pseudo_rand,
	int same_lane) {
	
	uint32_t reference_area_size;
	uint64_t relative_position;
	uint32_t start_position, absolute_position;

	if (0 == position->pass) {
		/* First pass */
		if (0 == position->slice) {
			/* First slice */
			reference_area_size =
				position->index - 1; /* all but the previous */
		}
		else {
			if (same_lane) {
				/* The same lane => add current segment */
				reference_area_size =
					position->slice * instance->segment_length +
					position->index - 1;
			}
			else {
				reference_area_size =
					position->slice * instance->segment_length +
					((position->index == 0) ? (-1) : 0);
			}
		}
	}
	else {
		/* Second pass */
		if (same_lane) {
			reference_area_size = instance->lane_length -
				instance->segment_length + position->index -
				1;
		}
		else {
			reference_area_size = instance->lane_length -
				instance->segment_length +
				((position->index == 0) ? (-1) : 0);
		}
	}

	/* 1.2.4. Mapping pseudo_rand to 0..<reference_area_size-1> and produce
	* relative position */
	relative_position = pseudo_rand;
	relative_position = relative_position * relative_position >> 32;
	relative_position = reference_area_size - 1 -
		(reference_area_size * relative_position >> 32);

	/* 1.2.5 Computing starting position */
	start_position = 0;

	if (0 != position->pass) {
		start_position = (position->slice == ARGON2_SYNC_POINTS - 1)
			? 0
			: (position->slice + 1) * instance->segment_length;
	}

	/* 1.2.6. Computing absolute position */
	absolute_position = (start_position + relative_position) %
		instance->lane_length; /* absolute position */
	return absolute_position;
}

void StoreBlock(void *output, const block *src)
{
	for (unsigned i = 0; i < ARGON2_QWORDS_IN_BLOCK; ++i) {
		store64(static_cast<uint8_t*>(output)
			+ (i * sizeof(src->v[i])), src->v[i]);
	}
}


void compute_blake2b(const block& input,
	uint8_t digest[MERKLE_TREE_ELEMENT_SIZE_B])
{
	ablake2b_state state;
	ablake2b_init(&state, MERKLE_TREE_ELEMENT_SIZE_B);
	ablake2b4rounds_update(&state, input.v, ARGON2_BLOCK_SIZE);
	ablake2b4rounds_final(&state, digest, MERKLE_TREE_ELEMENT_SIZE_B);
}


void getblockindex(uint32_t ij, argon2_instance_t *instance, uint32_t *out_ij_prev, uint32_t *out_computed_ref_block)
{
	uint32_t ij_prev = 0;
	if (ij%instance->lane_length == 0)
		ij_prev = ij + instance->lane_length - 1;
	else
		ij_prev = ij - 1;

	if (ij % instance->lane_length == 1)
		ij_prev = ij - 1;

	uint64_t prev_block_opening = instance->memory[ij_prev].v[0];
	uint32_t ref_lane = (uint32_t)((prev_block_opening >> 32) % instance->lanes);

	uint32_t pseudo_rand = (uint32_t)(prev_block_opening & 0xFFFFFFFF);

	uint32_t Lane = ((ij) / instance->lane_length);
	uint32_t Slice = (ij - (Lane * instance->lane_length)) / instance->segment_length;
	uint32_t posIndex = ij - Lane * instance->lane_length - Slice * instance->segment_length;


	uint32_t rec_ij = Slice*instance->segment_length + Lane *instance->lane_length + (ij % instance->segment_length);

	if (Slice == 0)
		ref_lane = Lane;


	argon2_position_t position = { 0, Lane , (uint8_t)Slice, posIndex };

	uint32_t ref_index = index_beta(instance, &position, pseudo_rand, ref_lane == position.lane);

	uint32_t computed_ref_block = instance->lane_length * ref_lane + ref_index;

	*out_ij_prev = ij_prev;
	*out_computed_ref_block = computed_ref_block;
}




unsigned int trailing_zeros(char str[64]) {


    unsigned int i, d;
    d = 0;
    for (i = 63; i > 0; i--) {
        if (str[i] == '0') {
            d++;
        }
        else {
            break;
        }
    }
    return d;
}


unsigned int trailing_zeros_little_endian(char str[64]) {
	unsigned int i, d;
	d = 0;
	for (i = 0; i < 64; i++) {
		if (str[i] == '0') {
			d++;
		}
		else {
			break;
		}
	}
	return d;
}

unsigned int trailing_zeros_little_endian_uint256(uint256 hash) {
	unsigned int i, d;
	std::string temp = hash.GetHex();
	d = 0;
	for (i = 0; i < temp.size(); i++) {
		if (temp[i] == '0') {
			d++;
		}
		else {
			break;
		}
	}
	return d;
}


static void store_block(void *output, const block *src) {
    unsigned i;
    for (i = 0; i < ARGON2_QWORDS_IN_BLOCK; ++i) {
        store64((uint8_t *)output + i * sizeof(src->v[i]), src->v[i]);
    }
}


void fill_block(__m128i *state, const block *ref_block, block *next_block, int with_xor) {
    __m128i block_XY[ARGON2_OWORDS_IN_BLOCK];
    unsigned int i;

    if (with_xor) {
        for (i = 0; i < ARGON2_OWORDS_IN_BLOCK; i++) {
            state[i] = _mm_xor_si128(
                    state[i], _mm_loadu_si128((const __m128i *)ref_block->v + i));
            block_XY[i] = _mm_xor_si128(
                    state[i], _mm_loadu_si128((const __m128i *)next_block->v + i));
        }
    }
    else {
        for (i = 0; i < ARGON2_OWORDS_IN_BLOCK; i++) {
            block_XY[i] = state[i] = _mm_xor_si128(
                    state[i], _mm_loadu_si128((const __m128i *)ref_block->v + i));
        }
    }

    for (i = 0; i < 8; ++i) {
        BLAKE2_ROUND(state[8 * i + 0], state[8 * i + 1], state[8 * i + 2],
                     state[8 * i + 3], state[8 * i + 4], state[8 * i + 5],
                     state[8 * i + 6], state[8 * i + 7]);
    }

    for (i = 0; i < 8; ++i) {
        BLAKE2_ROUND(state[8 * 0 + i], state[8 * 1 + i], state[8 * 2 + i],
                     state[8 * 3 + i], state[8 * 4 + i], state[8 * 5 + i],
                     state[8 * 6 + i], state[8 * 7 + i]);
    }

    for (i = 0; i < ARGON2_OWORDS_IN_BLOCK; i++) {
        state[i] = _mm_xor_si128(state[i], block_XY[i]);
        _mm_storeu_si128((__m128i *)next_block->v + i, state[i]);
    }
}

void fill_block2(__m128i *state, const block *ref_block, block *next_block, int with_xor, uint32_t block_header[4]) {
	__m128i block_XY[ARGON2_OWORDS_IN_BLOCK];
	unsigned int i;

	if (with_xor) {
		for (i = 0; i < ARGON2_OWORDS_IN_BLOCK; i++) {
			state[i] = _mm_xor_si128(
				state[i], _mm_loadu_si128((const __m128i *)ref_block->v + i));
			block_XY[i] = _mm_xor_si128(
				state[i], _mm_loadu_si128((const __m128i *)next_block->v + i));
		}
	}
	else {
		for (i = 0; i < ARGON2_OWORDS_IN_BLOCK; i++) {
			block_XY[i] = state[i] = _mm_xor_si128(
				state[i], _mm_loadu_si128((const __m128i *)ref_block->v + i));
		}
	}

	memcpy(&state[8], block_header, sizeof(__m128i));

	for (i = 0; i < 8; ++i) {
		BLAKE2_ROUND(state[8 * i + 0], state[8 * i + 1], state[8 * i + 2],
			state[8 * i + 3], state[8 * i + 4], state[8 * i + 5],
			state[8 * i + 6], state[8 * i + 7]);
	}

	for (i = 0; i < 8; ++i) {
		BLAKE2_ROUND(state[8 * 0 + i], state[8 * 1 + i], state[8 * 2 + i],
			state[8 * 3 + i], state[8 * 4 + i], state[8 * 5 + i],
			state[8 * 6 + i], state[8 * 7 + i]);
	}

	for (i = 0; i < ARGON2_OWORDS_IN_BLOCK; i++) {
		state[i] = _mm_xor_si128(state[i], block_XY[i]);
		_mm_storeu_si128((__m128i *)next_block->v + i, state[i]);
	}
}

void fill_block2_withIndex(__m128i *state, const block *ref_block, block *next_block, int with_xor, uint32_t block_header[8], uint64_t blockIndex) {
	__m128i block_XY[ARGON2_OWORDS_IN_BLOCK];
	unsigned int i;
    uint64_t TheIndex[2]={0,blockIndex};
	if (with_xor) {
		for (i = 0; i < ARGON2_OWORDS_IN_BLOCK; i++) {
			state[i] = _mm_xor_si128(
				state[i], _mm_loadu_si128((const __m128i *)ref_block->v + i));
			block_XY[i] = _mm_xor_si128(
				state[i], _mm_loadu_si128((const __m128i *)next_block->v + i));
		}
	}
	else {
		for (i = 0; i < ARGON2_OWORDS_IN_BLOCK; i++) {
			block_XY[i] = state[i] = _mm_xor_si128(
				state[i], _mm_loadu_si128((const __m128i *)ref_block->v + i));
		}
	}
	memcpy(&state[7], TheIndex, sizeof(__m128i));
	memcpy(&state[8], block_header, sizeof(__m128i));
	memcpy(&state[9], block_header + 4, sizeof(__m128i));
	for (i = 0; i < 8; ++i) {
		BLAKE2_ROUND(state[8 * i + 0], state[8 * i + 1], state[8 * i + 2],
			state[8 * i + 3], state[8 * i + 4], state[8 * i + 5],
			state[8 * i + 6], state[8 * i + 7]);
	}

	for (i = 0; i < 8; ++i) {
		BLAKE2_ROUND(state[8 * 0 + i], state[8 * 1 + i], state[8 * 2 + i],
			state[8 * 3 + i], state[8 * 4 + i], state[8 * 5 + i],
			state[8 * 6 + i], state[8 * 7 + i]);
	}

	for (i = 0; i < ARGON2_OWORDS_IN_BLOCK; i++) {
		state[i] = _mm_xor_si128(state[i], block_XY[i]);
		_mm_storeu_si128((__m128i *)next_block->v + i, state[i]);
	}
}



void copy_block(block *dst, const block *src) {
	memcpy(dst->v, src->v, sizeof(uint64_t) * ARGON2_QWORDS_IN_BLOCK);
}
void copy_blockS(blockS *dst, const blockS *src) {
	memcpy(dst->v, src->v, sizeof(uint64_t) * ARGON2_QWORDS_IN_BLOCK);
}
void copy_blockS(blockS *dst, const block *src) {
	memcpy(dst->v, src->v, sizeof(uint64_t) * ARGON2_QWORDS_IN_BLOCK);
}


#define VC_GE_2005(version) (version >= 1400)

void  secure_wipe_memory(void *v, size_t n) {
#if defined(_MSC_VER) && VC_GE_2005(_MSC_VER)
	SecureZeroMemory(v, n);
#elif defined memset_s
	memset_s(v, n, 0, n);
#elif defined(__OpenBSD__)
	explicit_bzero(v, n);
#else
	static void *(*const volatile memset_sec)(void *, int, size_t) = &memset;
	memset_sec(v, 0, n);
#endif
}

/* Memory clear flag defaults to true. */

void clear_internal_memory(void *v, size_t n) {
	if (FLAG_clear_internal_memory && v) {
		secure_wipe_memory(v, n);
	}
}


void free_memory(const argon2_context *context, uint8_t *memory,
	size_t num, size_t size) {
	size_t memory_size = num*size;
	clear_internal_memory(memory, memory_size);
	if (context->free_cbk) {
		(context->free_cbk)(memory, memory_size);
	}
	else {
		free(memory);
	}
}

argon2_context init_argon2d_param(const char* input) {

#define TEST_OUTLEN 32
#define TEST_PWDLEN 80
#define TEST_SALTLEN 80
#define TEST_SECRETLEN 0
#define TEST_ADLEN 0
    argon2_context context;
    argon2_context *pContext = &context;

    unsigned char out[TEST_OUTLEN];

    const allocate_fptr myown_allocator = NULL;
    const deallocate_fptr myown_deallocator = NULL;

    unsigned t_cost = 1;
    unsigned m_cost =  memcost; //2*1024*1024; //*1024; //+896*1024; //32768*1;
	
    unsigned lanes = 4;

    memset(pContext,0,sizeof(argon2_context));
    memset(&out[0], 0, sizeof(out));
    context.out = out;
    context.outlen = TEST_OUTLEN;
    context.version = ARGON2_VERSION_NUMBER;
    context.pwd = (uint8_t*)input;
    context.pwdlen = TEST_PWDLEN;
    context.salt = (uint8_t*)input;
    context.saltlen = TEST_SALTLEN;
    context.secret = NULL;
    context.secretlen = TEST_SECRETLEN;
    context.ad = NULL;
    context.adlen = TEST_ADLEN;
    context.t_cost = t_cost;
    context.m_cost = m_cost;
    context.lanes = lanes;
    context.threads = lanes;
    context.allocate_cbk = myown_allocator;
    context.free_cbk = myown_deallocator;
    context.flags = ARGON2_DEFAULT_FLAGS;

#undef TEST_OUTLEN
#undef TEST_PWDLEN
#undef TEST_SALTLEN
#undef TEST_SECRETLEN
#undef TEST_ADLEN

    return context;
}





int mtp_solver(uint32_t TheNonce, argon2_instance_t *instance,
	uint64_t nBlockMTP[MTP_BLOCK_PROOF_SIZE * 2][128] /*[72 * 2][128]*/, unsigned char* nProofMTP, unsigned char* resultMerkleRoot, unsigned char* mtpHashValue,
	MerkleTree TheTree, uint32_t* input, uint256 hashTarget) {



	if (instance != NULL) {
		//		input[19]=0x01000000;
		uint256 Y;
		//		std::string proof_blocks[L * 3];
		memset(&Y, 0, sizeof(Y));

		ablake2b_state BlakeHash;
		ablake2b_init(&BlakeHash, 32);


		ablake2b_update(&BlakeHash, (unsigned char*)&input[0], 80);
		ablake2b_update(&BlakeHash, (unsigned char*)&resultMerkleRoot[0], 16);
		ablake2b_update(&BlakeHash, &TheNonce, sizeof(unsigned int));
		ablake2b_final(&BlakeHash, (unsigned char*)&Y, 32);


		///////////////////////////////
		bool init_blocks = false;
		bool unmatch_block = false;

		for (int j = 1; j <= L; j++) {

			uint32_t ij = (((uint32_t*)(&Y))[0]) % (instance->context_ptr->m_cost);

			uint32_t except_index = (uint32_t)(instance->context_ptr->m_cost / instance->context_ptr->lanes);
			if (ij %except_index == 0 || ij%except_index == 1) {
				init_blocks = true;
				break;
			}

			uint32_t prev_index = 0;
			uint32_t ref_index = 0;
			getblockindex(ij, instance, &prev_index, &ref_index);




				for (int i = 0; i<128; i++)
					nBlockMTP[j*2-2][i] = instance->memory[prev_index].v[i];
				for (int i = 0; i<128; i++)
					nBlockMTP[j * 2 - 1][i] = instance->memory[ref_index].v[i];

			block blockhash;
			uint8_t blockhash_bytes[ARGON2_BLOCK_SIZE];
			copy_block(&blockhash, &instance->memory[ij]);


			store_block(&blockhash_bytes, &blockhash);

			ablake2b_state BlakeHash2;
			ablake2b_init(&BlakeHash2, 32);
			ablake2b_update(&BlakeHash2, &Y, sizeof(uint256));
			ablake2b_update(&BlakeHash2, blockhash_bytes, ARGON2_BLOCK_SIZE);
			ablake2b_final(&BlakeHash2, (unsigned char*)&Y, 32);
			////////////////////////////////////////////////////////////////
			// current block


			block blockhash_curr;
			uint8_t blockhash_curr_bytes[ARGON2_BLOCK_SIZE];
			copy_block(&blockhash_curr, &instance->memory[ij]);
			store_block(&blockhash_curr_bytes, &blockhash_curr);
			ablake2b_state state_curr;
			ablake2b_init(&state_curr, MERKLE_TREE_ELEMENT_SIZE_B);
			ablake2b4rounds_update(&state_curr, blockhash_curr_bytes, ARGON2_BLOCK_SIZE);
			uint8_t digest_curr[MERKLE_TREE_ELEMENT_SIZE_B];
			ablake2b4rounds_final(&state_curr, digest_curr, sizeof(digest_curr));
			MerkleTree::Buffer hash_curr = MerkleTree::Buffer(digest_curr, digest_curr + sizeof(digest_curr));
			clear_internal_memory(blockhash_curr.v, ARGON2_BLOCK_SIZE);
			clear_internal_memory(blockhash_curr_bytes, ARGON2_BLOCK_SIZE);


			std::deque<std::vector<uint8_t>> zProofMTP = TheTree.getProofOrdered(hash_curr, ij + 1);

			nProofMTP[(j * 3 - 3) * 353] = (unsigned char)(zProofMTP.size());

			int k1 = 0;
			for (const std::vector<uint8_t> &mtpData : zProofMTP) {
				std::copy(mtpData.begin(), mtpData.end(), nProofMTP + ((j * 3 - 3) * 353 + 1 + k1 * mtpData.size()));
				k1++;
			}

			//prev proof

			block blockhash_prev;
			uint8_t blockhash_prev_bytes[ARGON2_BLOCK_SIZE];
			copy_block(&blockhash_prev, &instance->memory[prev_index]);
			store_block(&blockhash_prev_bytes, &blockhash_prev);
			ablake2b_state state_prev;
			ablake2b_init(&state_prev, MERKLE_TREE_ELEMENT_SIZE_B);
			ablake2b4rounds_update(&state_prev, blockhash_prev_bytes, ARGON2_BLOCK_SIZE);
			uint8_t digest_prev[MERKLE_TREE_ELEMENT_SIZE_B];


			ablake2b4rounds_final(&state_prev, digest_prev, sizeof(digest_prev));


			MerkleTree::Buffer hash_prev = MerkleTree::Buffer(digest_prev, digest_prev + sizeof(digest_prev));
			clear_internal_memory(blockhash_prev.v, ARGON2_BLOCK_SIZE);
			clear_internal_memory(blockhash_prev_bytes, ARGON2_BLOCK_SIZE);

			std::deque<std::vector<uint8_t>> zProofMTP2 = TheTree.getProofOrdered(hash_prev, prev_index + 1);

			nProofMTP[(j * 3 - 2) * 353] = (unsigned char)(zProofMTP2.size());

			int k2 = 0;
			for (const std::vector<uint8_t> &mtpData : zProofMTP2) {
				std::copy(mtpData.begin(), mtpData.end(), nProofMTP + ((j * 3 - 2) * 353 + 1 + k2 * mtpData.size()));
				k2++;
			}


			//ref proof

			block blockhash_ref;
			uint8_t blockhash_ref_bytes[ARGON2_BLOCK_SIZE];
			copy_block(&blockhash_ref, &instance->memory[ref_index]);
			store_block(&blockhash_ref_bytes, &blockhash_ref);
			ablake2b_state state_ref;
			ablake2b_init(&state_ref, MERKLE_TREE_ELEMENT_SIZE_B);
			ablake2b4rounds_update(&state_ref, blockhash_ref_bytes, ARGON2_BLOCK_SIZE);
			uint8_t digest_ref[MERKLE_TREE_ELEMENT_SIZE_B];
			ablake2b4rounds_final(&state_ref, digest_ref, sizeof(digest_ref));
			MerkleTree::Buffer hash_ref = MerkleTree::Buffer(digest_ref, digest_ref + sizeof(digest_ref));
			clear_internal_memory(blockhash_ref.v, ARGON2_BLOCK_SIZE);
			clear_internal_memory(blockhash_ref_bytes, ARGON2_BLOCK_SIZE);

			std::deque<std::vector<uint8_t>> zProofMTP3 = TheTree.getProofOrdered(hash_ref, ref_index + 1);

			nProofMTP[(j * 3 - 1) * 353] = (unsigned char)(zProofMTP3.size());

			int k3 = 0;
			for (const std::vector<uint8_t> &mtpData : zProofMTP3) {
				std::copy(mtpData.begin(), mtpData.end(), nProofMTP + ((j * 3 - 1) * 353 + 1 + k3 * mtpData.size()));
				k3++;
			}

		}

		if (init_blocks) {

			return 0;
		}


		char hex_tmp[64];

		if (Y > hashTarget) {

		}
		else {
			for (int i = 0; i<32; i++)
				mtpHashValue[i] = (((unsigned char*)(&Y))[i]);

			// Found a solution
			printf("Found a solution. Nonce=%08x Hash:", TheNonce);
			printf("\n");
			return 1;


		}

	}


	return 0;
}


int mtp_solver_nowriting(uint32_t TheNonce, argon2_instance_t *instance,
	unsigned char* resultMerkleRoot, uint32_t* input, uint256 hashTarget) {

	if (instance != NULL) {
		uint256 Y; 
//		memset(&Y, 0, sizeof(Y));

		ablake2b_state BlakeHash;
		ablake2b_init(&BlakeHash, 32);
		ablake2b_update(&BlakeHash, (unsigned char*)&input[0], 80);
		ablake2b_update(&BlakeHash, (unsigned char*)&resultMerkleRoot[0], 16);
		ablake2b_update(&BlakeHash, &TheNonce, sizeof(unsigned int));
		ablake2b_final(&BlakeHash, (unsigned char*)&Y, 32);

		///////////////////////////////
		bool init_blocks = false;
		bool unmatch_block = false;


		for (uint8_t j = 1; j <= L; j++) {

			uint32_t ij = (((uint32_t*)(&Y))[0]) % (instance->context_ptr->m_cost);

			uint32_t except_index = (uint32_t)(instance->context_ptr->m_cost / instance->context_ptr->lanes);
			if (ij %except_index == 0 || ij%except_index == 1) {
				init_blocks = true;
				break;
			}

			unsigned char TheBlock[1024];
			memcpy(TheBlock, instance->memory[ij].v,1024);
			ablake2b_init(&BlakeHash, 32);
			ablake2b_update(&BlakeHash, &Y, sizeof(uint256));
			ablake2b_update(&BlakeHash, TheBlock, ARGON2_BLOCK_SIZE);
			ablake2b_final(&BlakeHash, (unsigned char*)&Y, 32);

		}

		if (init_blocks) 
					return 0;

		if (Y <= hashTarget) 
					return 1;

	}
	return 0;
}





void mtp_init( argon2_instance_t *instance,MerkleTree::Elements  *elements) {

	printf("Step 1 : Compute F(I) and store its T blocks X[1], X[2], ..., X[T] in the memory \n");
//	MerkleTree::Elements elements;
	if (instance != NULL) {
		printf("Step 2 : Compute the root Φ of the Merkle hash tree \n");

		for (long int i = 0; i < instance->memory_blocks; ++i) {
			uint8_t digest[MERKLE_TREE_ELEMENT_SIZE_B];
			compute_blake2b(instance->memory[i], digest);
			elements->emplace_back(digest, digest + sizeof(digest));
//			elements->push_back(digest, digest + sizeof(digest));
		}

		printf("end Step 2 : Compute the root Φ of the Merkle hash tree \n");
//		return elements;
	}

}

MerkleTree::Elements   mtp_init2(argon2_instance_t *instance) {

	MerkleTree::Elements  elements;
	printf("Step 1 : Compute F(I) and store its T blocks X[1], X[2], ..., X[T] in the memory \n");
	//	MerkleTree::Elements elements;
	if (instance != NULL) {
		printf("Step 2 : Compute the root Φ of the Merkle hash tree \n");
		uint8_t digest[MERKLE_TREE_ELEMENT_SIZE_B];
		for (int i = 0; i < instance->memory_blocks/2; ++i) {
			memset(digest,0,MERKLE_TREE_ELEMENT_SIZE_B);
			compute_blake2b(instance->memory[2*i], digest);
			elements.emplace_back(digest, digest + sizeof(digest));
			memset(digest, 0, MERKLE_TREE_ELEMENT_SIZE_B);
			compute_blake2b(instance->memory[2*i+1], digest);
			elements.emplace_back(digest, digest + sizeof(digest));
//			elements->push_back(digest, digest + sizeof(digest));
		}

		printf("end Step 2 : Compute the root Φ of the Merkle hash tree \n");
		return elements;
	}

}

//
void mtp_hash(char* output, const char* input, unsigned int d,uint32_t TheNonce) {
    argon2_context context = init_argon2d_param(input);
    argon2_instance_t instance;
    argon2_ctx_from_mtp(&context, &instance);
//    mtp_prover(TheNonce, &instance, d, output);
//    free_memory(&context, (uint8_t *)instance.memory, instance.memory_blocks, sizeof(block));

}