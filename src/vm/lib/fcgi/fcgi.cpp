/***************************************************************************
 * FastCGI support for Objeck
 *
 * Copyright (c) 2011-2012, Randy Hollines
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the distribution.
 * - Neither the name of the Objeck Team nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

#include "fcgi_stdio.h"
#include "fcgi_stdio.h"

#include "../../lib_api.h"

extern "C" {
  static void fcgi_get_env_value(const char* name, VMContext& context);
    
  //
  // initialize fcgi environment
  //
#ifdef _WIN32
  __declspec(dllexport) 
#endif
  void load_lib() {
  }
  
  //
  // release fcgi resources
  //
#ifdef _WIN32
  __declspec(dllexport) 
#endif
  void unload_lib() {
  }
  
  //
  // TOOD
  //
#ifdef _WIN32
  __declspec(dllexport) 
#endif
  void fcgi_write(VMContext& context) {
    FCGX_Stream* out = (FCGX_Stream*)APITools_GetIntValue(context, 0);
    const char* value = APITools_GetStringValue(context, 1);
    if(out && value) {
      FCGX_PutS(value, out);
    }
  }
  
  //
  // TOOD
  //
#ifdef _WIN32
  __declspec(dllexport) 
#endif
  void fcgi_get_args(VMContext& context) {
  }
  
  //
  // TOOD
  //
#ifdef _WIN32
  __declspec(dllexport) 
#endif
  void fcgi_get_protocol(VMContext& context) {
    fcgi_get_env_value("SERVER_PROTOCOL", context);
  }
  
  void fcgi_get_query(VMContext& context) {
    fcgi_get_env_value("QUERY_STRING", context);
  }

  void fcgi_get_cookie(VMContext& context) {
    fcgi_get_env_value("HTTP_COOKIE", context);
  }
  
  void fcgi_get_remote_address(VMContext& context) {
    fcgi_get_env_value("REMOTE_ADDR", context);
  }
  
  void fcgi_get_request_method(VMContext& context) {
    fcgi_get_env_value("REQUEST_METHOD", context);
  }

  void fcgi_get_request_uri(VMContext& context) {
    fcgi_get_env_value("REQUEST_URI", context);
  }
  
  static void fcgi_get_env_value(const char* name, VMContext& context) {
    FCGX_ParamArray envp = (FCGX_ParamArray)APITools_GetIntValue(context, 0);
    if(envp) {
      char* value = FCGX_GetParam(name, envp);
      if(value) {
	APITools_SetStringValue(context, 1, value);
	return;
      }
    }
    
    APITools_SetStringValue(context, 1, "");
  }
}


