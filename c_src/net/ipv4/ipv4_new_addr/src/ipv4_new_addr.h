/*=============================================================================

Copyright (c) 2013, Naoto Uegaki
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#ifndef _IPV4_NEW_ADDR_H_
#define _IPV4_NEW_ADDR_H_

//=============================================================================
// Includes
//=============================================================================
#include <stdint.h>


//=============================================================================
// Macro/Type/Enumeration/Structure Definitions
//=============================================================================

/*!
  \brief
  "ipv4_new_addr_cb_t" is the type of callback function to be
  called when ip address is changed.

  \param[in] err_code
  0 for success, -1 for error.
  If error happened, this callback should be ignored, and the descriptor should be closed.

  \param[in] new_addr
  The new IP address.
  Particularly, when opened, the callback will be called to notify the first new_addr.

  \param[in,out] p_cookie
  The pointer to the user data.

  \return
  NULL for success, and the other values for error.
*/
typedef int (*ipv4_new_addr_cb_t)(int err_code,
                                  uint32_t new_addr,
                                  void *p_cookie);


//=============================================================================
// Global Function/Variable Prototypes
//=============================================================================

/*!
  \brief
  "ipv4_new_addr_open()" open a module of detecting IP address changing.
  When changed, fucn will be executed.

  \param[in] p_ifname
  The network interface name to detect IP address changing.

  \param[in] ipv4_new_addr_cb_t
  The callback function to be executed when IP address changed.

  \param[in] polling_interval
  The poling interval.
  
  \param[in] p_cookie
  The pointer to the user data.

  \return
  It returns a descriptor. If error happened, -1 will return.
*/
int ipv4_new_addr_open(const char *p_ifname,
                       const ipv4_new_addr_cb_t func,
                       int polling_interval,
                       const void *p_cookie);


/*!
  \brief
  "ipv4_new_addr_close()" close a module of detecting IP address changing.

  \param[in] fd
  The descriptor.

  \return
  0 for success, and -1 for error.
*/
int ipv4_new_addr_close(int fd);


#endif // _IPV4_NEW_ADDR_H_
