/*
 * Copyright (c) 2004-2005 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Erik Hallnor
 *          Steve Reinhardt
 *          Lisa Hsu
 *          Kevin Lim
 */

/**
 * @file
 * Cache template instantiations.
 */

#include "mem/cache/tags/fa_lru.hh"
#include "mem/cache/tags/lru.hh"
#include "mem/cache/tags/random_repl.hh"
//light
#include "mem/cache/tags/pipp.hh"
#include "mem/cache/tags/feaf.hh"
#include "mem/cache/tags/hap.hh"
#include "mem/cache/tags/heaf.hh"
#include "mem/cache/tags/hap_test.hh"
#include "mem/cache/tags/hyblru.hh"
#include "mem/cache/tags/hybbf.hh"
#include "mem/cache/tags/hybdip.hh"
#include "mem/cache/tags/ihybdip.hh"
#include "mem/cache/tags/lru_wb.hh"
#include "mem/cache/tags/hybrrip.hh"
//end
//xiang
#include "mem/cache/tags/brp.hh"
//end
#include "mem/cache/cache_impl.hh"

// Template Instantiations
#ifndef DOXYGEN_SHOULD_SKIP_THIS

//template class Cache<FALRU>;
template class Cache<LRU>;
template class Cache<RandomRepl>;
//light
template class Cache<PIPP>;
template class Cache<FEAF>;
template class Cache<HAP>;
template class Cache<HEAF>;
template class Cache<HAPTEST>;
template class Cache<HYBLRU>;
template class Cache<HYBBF>;
template class Cache<HYBDIP>;
template class Cache<IHYBDIP>;
template class Cache<LRUWB>;
template class Cache<HYBRRIP>;
//end
//xiang
template class Cache<BRP>;
//end
#endif //DOXYGEN_SHOULD_SKIP_THIS
