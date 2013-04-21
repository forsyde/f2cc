/**
@mainpage

\b f2cc is a software synthesis tool developed as part of the <a
href="http://www.ict.kth.se/forsyde/">ForSyDe</a> (Formal System Design)
project. The name stands for "ForSyDe-2-CUDA C" and enables models to be
synthesized into either C code or CUDA C code, the latter which can be compiled
for parallel execution on a CUDA-enabled NVIDIA graphics card. Currently, the
tool can handle models which contain the following leaf types:
   - \c f2cc::ForSyDe::SY::comb
   - \c f2cc::ForSyDe::SY::ParallelMap
   - \c f2cc::ForSyDe::SY::unzipx
   - \c f2cc::ForSyDe::SY::zipx
   - \c f2cc::ForSyDe::SY::delay

The tool recognizes simple data parallel patterns in a processnetwork (an \c unzipx
leaf, followed by a series of \c comb leafs, which all connect to a \c
zipx leaf) and generate correct CUDA code for such regions. Support for
additional leaf types and data parallel processnetwork patterns will be added in the
future.

\b f2cc was originally developed by Gabriel Hjort Blindell as part of his Master
thesis at KTH (Royal Institute of Technology), Sweden. For more information
about f2cc, the reader is advised to consult the thesis report.

For licensing and copyright information, see the \ref license "License" page.



@page license License

f2cc is licensed under the BSD 2-clause license.

Copyright &copy; 2011-2012 Gabriel Hjort Blindell &lt;ghb@kth.se&gt;
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    - Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OF THIS SOFTWARE NOR THE COPYRIGHT
HOLDERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
