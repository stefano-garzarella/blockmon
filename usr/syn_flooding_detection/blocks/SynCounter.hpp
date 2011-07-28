/* Copyright (c) 2011, NEC Europe Ltd, Consorzio Nazionale 
 * Interuniversitario per le Telecomunicazioni, Institut 
 * Telecom/Telecom Bretagne, ETH Zürich, INVEA-TECH a.s. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the names of NEC Europe Ltd, Consorzio Nazionale 
 *      Interuniversitario per le Telecomunicazioni, Institut Telecom/Telecom 
 *      Bretagne, ETH Zürich, INVEA-TECH a.s. nor the names of its contributors 
 *      may be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT 
 * HOLDERBE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 */

/**
 * @file
 * Counts the TCP SYN packets received from each IP. Store the counts in a CMS sketch
 * and send the sketch regularly.
 * The sketch configuration is received from a synchronization block.
 * Receive the SYN flooding alerts from the detector and find the responsible IP (if any)
 * Configuration:
 *	<export period="500000" />
 * The period is the number of microseconds between each sketch is sent.
 *
 * @blockname{SynCounter}
 * @gates{in_sketch_proto(msg:Sketch), in_alert(msg:CmsSignature), in_pkt(msg:TuplePacket), out_sketch(msg:Sketch)}
 *
 */

#include <Block.hpp>
#include <BlockFactory.hpp>
#include <cms.hpp>

#define SYF_LAST_IPS_MAX 100
#define SYF_TCP_FLAGS_LOCATION 13
#define SYF_TCP_FLAGS_ACK 0x10
#define SYF_TCP_FLAGS_SYN 0x02

namespace bm
{

    /**
     * Implements a block that counts the received packets by flow
     */
    class SynCounter: public Block
    {
        int m_ingate_proto_id;
        int m_ingate_alert_id;
        int m_ingate_id;
        int m_outgate_id;
        std::shared_ptr<cms::Sketch> m_sketch;
        bool m_sketch_ready;
		unsigned int m_last_ips[SYF_LAST_IPS_MAX];
		int m_last_ip_index;
		int m_last_ip_count;

        // Configuration
        int m_period;

    public:

        /**
         * @brief Constructor
         * @param name         The name of the CMS packet counter block
         * @param active       Whether the block should be active or passive
         * @param thread_safe  Whether thread-safety should be enabled
         */
        SynCounter(const std::string &name, bool active, bool thread_safe);

        /**
         * @brief Destructor
         */
        virtual ~SynCounter();

        /** 
          * Non copyable by default: note m_sketch is a pointer, proper copy constructor and op= should be provided. Nicola  
          */
        SynCounter(const SynCounter &) = delete;
        SynCounter& operator=(const SynCounter &) = delete;

        /**
         * @brief Configures the block
         * @param n The configuration parameters 
         */
        virtual void _configure(const xml_node&  n);

        /**
         * If the message received is not of type RawPacket throw an exception,
         * otherwise count it
         * @param m     The message
         * @param index The index of the gate the message came on
         * @return      0 upon success, negative otherwise
         */
        virtual int _receive_msg(std::shared_ptr<const Msg>&& m, int index);

        /**
         * Performs any asynchronous processing (no-op function)
         * @return 0 upon success, negative otherwise
         */
        virtual int do_async_processing()
        {
            return 0;
        }

        /**
         * Timer call back, export the sketch
         * @return 0 upon success, negative otherwise
         */
        virtual int _handle_timer(std::shared_ptr<Timer>&&);
    };


    REGISTER_BLOCK(SynCounter, "syn_counter");

}//bm