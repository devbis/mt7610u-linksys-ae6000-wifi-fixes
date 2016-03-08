/*
 *************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                       *
 *************************************************************************/


#include "rt_config.h"

UCHAR CipherWpaTemplate[] =
{
    0xdd,			/* WPA IE */
    0x16,			/* Length */
    0x00, 0x50, 0xf2, 0x01,	/* oui */
    0x01, 0x00,		/* Version */
    0x00, 0x50, 0xf2, 0x02,	/* Multicast */
    0x01, 0x00,		/* Number of unicast */
    0x00, 0x50, 0xf2, 0x02,	/* unicast */
    0x01, 0x00,		/* number of authentication method */
    0x00, 0x50, 0xf2, 0x01	/* authentication */
};

UCHAR CipherWpa2Template[] =
{
    0x30,			/* RSN IE */
    0x14,			/* Length */
    0x01, 0x00,		/* Version */
    0x00, 0x0f, 0xac, 0x02,	/* group cipher, TKIP */
    0x01, 0x00,		/* number of pairwise */
    0x00, 0x0f, 0xac, 0x02,	/* unicast */
    0x01, 0x00,		/* number of authentication method */
    0x00, 0x0f, 0xac, 0x02,	/* authentication */
    0x00, 0x00,		/* RSN capability */
};

/*
	==========================================================================
	Description:
		association state machine init, including state transition and timer init
	Parameters:
		S - pointer to the association state machine

	IRQL = PASSIVE_LEVEL

	==========================================================================
 */
VOID AssocStateMachineInit(
    IN PRTMP_ADAPTER pAd,
    IN STATE_MACHINE *S,
    OUT STATE_MACHINE_FUNC Trans[])
{
    StateMachineInit(S, Trans, MAX_ASSOC_STATE, MAX_ASSOC_MSG,
                     (STATE_MACHINE_FUNC) Drop, ASSOC_IDLE,
                     ASSOC_MACHINE_BASE);

    /* first column */
    StateMachineSetAction(S, ASSOC_IDLE, MT2_MLME_ASSOC_REQ,
                          (STATE_MACHINE_FUNC) MlmeAssocReqAction);
    StateMachineSetAction(S, ASSOC_IDLE, MT2_MLME_REASSOC_REQ,
                          (STATE_MACHINE_FUNC) MlmeReassocReqAction);
    StateMachineSetAction(S, ASSOC_IDLE, MT2_MLME_DISASSOC_REQ,
                          (STATE_MACHINE_FUNC) MlmeDisassocReqAction);
    StateMachineSetAction(S, ASSOC_IDLE, MT2_PEER_DISASSOC_REQ,
                          (STATE_MACHINE_FUNC) PeerDisassocAction);

    /* second column */
    StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_MLME_ASSOC_REQ,
                          (STATE_MACHINE_FUNC) InvalidStateWhenAssoc);
    StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_MLME_REASSOC_REQ,
                          (STATE_MACHINE_FUNC) InvalidStateWhenReassoc);
    StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_MLME_DISASSOC_REQ,
                          (STATE_MACHINE_FUNC)
                          InvalidStateWhenDisassociate);
    StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_PEER_DISASSOC_REQ,
                          (STATE_MACHINE_FUNC) PeerDisassocAction);
    StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_PEER_ASSOC_RSP,
                          (STATE_MACHINE_FUNC) PeerAssocRspAction);
    /* */
    /* Patch 3Com AP MOde:3CRWE454G72 */
    /* We send Assoc request frame to this AP, it always send Reassoc Rsp not Associate Rsp. */
    /* */
    StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_PEER_REASSOC_RSP,
                          (STATE_MACHINE_FUNC) PeerAssocRspAction);
    StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_ASSOC_TIMEOUT,
                          (STATE_MACHINE_FUNC) AssocTimeoutAction);

    /* third column */
    StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_MLME_ASSOC_REQ,
                          (STATE_MACHINE_FUNC) InvalidStateWhenAssoc);
    StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_MLME_REASSOC_REQ,
                          (STATE_MACHINE_FUNC) InvalidStateWhenReassoc);
    StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_MLME_DISASSOC_REQ,
                          (STATE_MACHINE_FUNC)
                          InvalidStateWhenDisassociate);
    StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_PEER_DISASSOC_REQ,
                          (STATE_MACHINE_FUNC) PeerDisassocAction);
    StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_PEER_REASSOC_RSP,
                          (STATE_MACHINE_FUNC) PeerReassocRspAction);
    /* */
    /* Patch, AP doesn't send Reassociate Rsp frame to Station. */
    /* */
    StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_PEER_ASSOC_RSP,
                          (STATE_MACHINE_FUNC) PeerReassocRspAction);
    StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_REASSOC_TIMEOUT,
                          (STATE_MACHINE_FUNC) ReassocTimeoutAction);

    /* fourth column */
    StateMachineSetAction(S, DISASSOC_WAIT_RSP, MT2_MLME_ASSOC_REQ,
                          (STATE_MACHINE_FUNC) InvalidStateWhenAssoc);
    StateMachineSetAction(S, DISASSOC_WAIT_RSP, MT2_MLME_REASSOC_REQ,
                          (STATE_MACHINE_FUNC) InvalidStateWhenReassoc);
    StateMachineSetAction(S, DISASSOC_WAIT_RSP, MT2_MLME_DISASSOC_REQ,
                          (STATE_MACHINE_FUNC)
                          InvalidStateWhenDisassociate);
    StateMachineSetAction(S, DISASSOC_WAIT_RSP, MT2_PEER_DISASSOC_REQ,
                          (STATE_MACHINE_FUNC) PeerDisassocAction);
    StateMachineSetAction(S, DISASSOC_WAIT_RSP, MT2_DISASSOC_TIMEOUT,
                          (STATE_MACHINE_FUNC) DisassocTimeoutAction);

    /* initialize the timer */
    RTMPInitTimer(pAd, &pAd->MlmeAux.AssocTimer,
                  GET_TIMER_FUNCTION(AssocTimeout), pAd, FALSE);
    RTMPInitTimer(pAd, &pAd->MlmeAux.ReassocTimer,
                  GET_TIMER_FUNCTION(ReassocTimeout), pAd, FALSE);
    RTMPInitTimer(pAd, &pAd->MlmeAux.DisassocTimer,
                  GET_TIMER_FUNCTION(DisassocTimeout), pAd, FALSE);
}

/*
	==========================================================================
	Description:
		Association timeout procedure. After association timeout, this function
		will be called and it will put a message into the MLME queue
	Parameters:
		Standard timer parameters

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AssocTimeout(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3)
{
    RTMP_ADAPTER *pAd = (RTMP_ADAPTER *) FunctionContext;

    /* Do nothing if the driver is starting halt state. */
    /* This might happen when timer already been fired before cancel timer with mlmehalt */
    if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
        return;

    MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_ASSOC_TIMEOUT, 0, NULL, 0);
    RTMP_MLME_HANDLER(pAd);
}

/*
	==========================================================================
	Description:
		Reassociation timeout procedure. After reassociation timeout, this
		function will be called and put a message into the MLME queue
	Parameters:
		Standard timer parameters

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID ReassocTimeout(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3)
{
    RTMP_ADAPTER *pAd = (RTMP_ADAPTER *) FunctionContext;

    /* Do nothing if the driver is starting halt state. */
    /* This might happen when timer already been fired before cancel timer with mlmehalt */
    if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
        return;

    MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_REASSOC_TIMEOUT, 0, NULL, 0);
    RTMP_MLME_HANDLER(pAd);
}

/*
	==========================================================================
	Description:
		Disassociation timeout procedure. After disassociation timeout, this
		function will be called and put a message into the MLME queue
	Parameters:
		Standard timer parameters

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID DisassocTimeout(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3)
{
    RTMP_ADAPTER *pAd = (RTMP_ADAPTER *) FunctionContext;

    /* Do nothing if the driver is starting halt state. */
    /* This might happen when timer already been fired before cancel timer with mlmehalt */
    if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
        return;

    MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_DISASSOC_TIMEOUT, 0, NULL, 0);
    RTMP_MLME_HANDLER(pAd);
}

/*
	==========================================================================
	Description:
		mlme assoc req handling procedure
	Parameters:
		Adapter - Adapter pointer
		Elem - MLME Queue Element
	Pre:
		the station has been authenticated and the following information is stored in the config
			-# SSID
			-# supported rates and their length
			-# listen interval (Adapter->StaCfg.default_listen_count)
			-# Transmit power  (Adapter->StaCfg.tx_power)
	Post  :
		-# An association request frame is generated and sent to the air
		-# Association timer starts
		-# Association state -> ASSOC_WAIT_RSP

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MlmeAssocReqAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    UCHAR ApAddr[6];
    HEADER_802_11 AssocHdr;
    UCHAR WmeIe[9] = {IE_VENDOR_SPECIFIC, 0x07, 0x00, 0x50, 0xf2, 0x02, 0x00, 0x01, 0x00};
    USHORT ListenIntv;
    ULONG Timeout;
    USHORT CapabilityInfo;
    BOOLEAN TimerCancelled;
    PUCHAR pOutBuffer = NULL;
    NDIS_STATUS NStatus;
    ULONG FrameLen = 0;
    ULONG tmp;
    USHORT VarIesOffset = 0;
    USHORT Status;


#ifdef WPA_SUPPLICANT_SUPPORT
    DBGPRINT(RT_DEBUG_TRACE, ("wpa_supplicant support enabled\n"));
#endif

#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
    DBGPRINT(RT_DEBUG_TRACE, ("Native wpa_supplicant support enabled\n"));
#endif

    /* Block all authentication request durning WPA block period */
    if(pAd->StaCfg.bBlockAssoc == TRUE)
    {
        DBGPRINT(RT_DEBUG_TRACE,
                 ("ASSOC - Block Assoc request durning WPA block period!\n"));
        pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
        Status = MLME_STATE_MACHINE_REJECT;
        MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2,
                    &Status, 0);
    }
    /* check sanity first */
    else if(MlmeAssocReqSanity(pAd, Elem->Msg, Elem->MsgLen, ApAddr, &CapabilityInfo, &Timeout, &ListenIntv))
    {
        /*for dhcp,issue ,wpa_supplicant ioctl too fast , at link_up, it will add key before driver remove key  */
        RTMPWPARemoveAllKeys(pAd);

        RTMPCancelTimer(&pAd->MlmeAux.AssocTimer, &TimerCancelled);
        COPY_MAC_ADDR(pAd->MlmeAux.Bssid, ApAddr);

        /* Get an unused nonpaged memory */
        NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

        if(NStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_TRACE,
                     ("ASSOC - MlmeAssocReqAction() allocate memory failed\n"));
            pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
            Status = MLME_FAIL_NO_RESOURCE;
            MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE,
                        MT2_ASSOC_CONF, 2, &Status, 0);
            return;
        }

        /* Add by James 03/06/27 */
        pAd->StaCfg.AssocInfo.Length =
            sizeof(NDIS_802_11_ASSOCIATION_INFORMATION);
        /* Association don't need to report MAC address */
        pAd->StaCfg.AssocInfo.AvailableRequestFixedIEs =
            NDIS_802_11_AI_REQFI_CAPABILITIES | NDIS_802_11_AI_REQFI_LISTENINTERVAL;
        pAd->StaCfg.AssocInfo.RequestFixedIEs.Capabilities = CapabilityInfo;
        pAd->StaCfg.AssocInfo.RequestFixedIEs.ListenInterval = ListenIntv;
        /* Only reassociate need this */
        /*COPY_MAC_ADDR(pAd->StaCfg.AssocInfo.RequestFixedIEs.CurrentAPAddress, ApAddr); */
        pAd->StaCfg.AssocInfo.OffsetRequestIEs = sizeof(NDIS_802_11_ASSOCIATION_INFORMATION);

        NdisZeroMemory(pAd->StaCfg.ReqVarIEs, MAX_VIE_LEN);
        /* First add SSID */
        VarIesOffset = 0;
        NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, &SsidIe, 1);
        VarIesOffset += 1;
        NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, &pAd->MlmeAux.SsidLen, 1);
        VarIesOffset += 1;
        NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
        VarIesOffset += pAd->MlmeAux.SsidLen;

        /* Second add Supported rates */
        NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, &SupRateIe, 1);
        VarIesOffset += 1;
        NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, &pAd->MlmeAux.SupRateLen, 1);
        VarIesOffset += 1;
        NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, pAd->MlmeAux.SupRate, pAd->MlmeAux.SupRateLen);
        VarIesOffset += pAd->MlmeAux.SupRateLen;
        /* End Add by James */

        DBGPRINT(RT_DEBUG_INFO, ("ASSOC - Send ASSOC request...\n"));
        MgtMacHeaderInit(pAd, &AssocHdr, SUBTYPE_ASSOC_REQ, 0, ApAddr,
                         ApAddr);

        /* Build basic frame first */
        MakeOutgoingFrame(pOutBuffer, &FrameLen,
                          sizeof(HEADER_802_11), &AssocHdr,
                          2, &CapabilityInfo,
                          2, &ListenIntv,
                          1, &SsidIe,
                          1, &pAd->MlmeAux.SsidLen,
                          pAd->MlmeAux.SsidLen, pAd->MlmeAux.Ssid,
                          1, &SupRateIe,
                          1, &pAd->MlmeAux.SupRateLen,
                          pAd->MlmeAux.SupRateLen, pAd->MlmeAux.SupRate,
                          END_OF_ARGS);

        if(pAd->MlmeAux.ExtRateLen != 0)
        {
            MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
                              1, &ExtRateIe,
                              1, &pAd->MlmeAux.ExtRateLen,
                              pAd->MlmeAux.ExtRateLen,
                              pAd->MlmeAux.ExtRate, END_OF_ARGS);
            FrameLen += tmp;
        }

#ifdef DOT11_N_SUPPORT

        /* HT */
        if((pAd->MlmeAux.HtCapabilityLen > 0)
                && WMODE_CAP_N(pAd->CommonCfg.PhyMode))
        {
            ULONG TmpLen;
            UCHAR HtLen, BROADCOM[4] = { 0x0, 0x90, 0x4c, 0x33 };
            PHT_CAPABILITY_IE pHtCapability;
#ifdef RT_BIG_ENDIAN
            HT_CAPABILITY_IE HtCapabilityTmp;
            NdisZeroMemory(&HtCapabilityTmp, sizeof(HT_CAPABILITY_IE));
            NdisMoveMemory(&HtCapabilityTmp, &pAd->MlmeAux.HtCapability, pAd->MlmeAux.HtCapabilityLen);
            *(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
            *(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
            pHtCapability = &HtCapabilityTmp;
#else
            pHtCapability = &pAd->MlmeAux.HtCapability;
#endif

            if(pAd->StaActive.SupportedPhyInfo.bPreNHt == TRUE)
            {
                HtLen = SIZE_HT_CAP_IE + 4;
                MakeOutgoingFrame(pOutBuffer + FrameLen,
                                  &TmpLen, 1, &WpaIe, 1, &HtLen,
                                  4, &BROADCOM[0],
                                  pAd->MlmeAux.HtCapabilityLen,
                                  pHtCapability, END_OF_ARGS);
            }
            else
            {
                MakeOutgoingFrame(pOutBuffer + FrameLen,
                                  &TmpLen, 1, &HtCapIe, 1,
                                  &pAd->MlmeAux.HtCapabilityLen,
                                  pAd->MlmeAux.HtCapabilityLen,
                                  pHtCapability, END_OF_ARGS);
            }

            FrameLen += TmpLen;

#ifdef DOT11_VHT_AC

            if(WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
                    (pAd->MlmeAux.Channel > 14) &&
                    (pAd->MlmeAux.vht_cap_len)
              )
            {
                FrameLen += build_vht_ies(pAd, (UCHAR *)(pOutBuffer + FrameLen), SUBTYPE_ASSOC_REQ);
            }

#endif /* DOT11_VHT_AC */
        }

#endif /* DOT11_N_SUPPORT */

#if defined(DOT11N_DRAFT3) || defined(DOT11V_WNM_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT)
        {
            ULONG TmpLen;
            EXT_CAP_INFO_ELEMENT extCapInfo;
            UCHAR extInfoLen;

            extInfoLen = sizeof(EXT_CAP_INFO_ELEMENT);
            NdisZeroMemory(&extCapInfo, extInfoLen);

#ifdef DOT11N_DRAFT3

            if((pAd->CommonCfg.bBssCoexEnable == TRUE) &&
                    WMODE_CAP_N(pAd->CommonCfg.PhyMode)
                    && (pAd->CommonCfg.Channel <= 14)
              )
            {
                extCapInfo.BssCoexistMgmtSupport = 1;
            }

#endif /* DOT11N_DRAFT3 */


            MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
                              1, &ExtCapIe,
                              1, &extInfoLen,
                              extInfoLen,			&extCapInfo,
                              END_OF_ARGS);
            FrameLen += TmpLen;
        }
#endif /* defined(DOT11N_DRAFT3) || defined(DOT11V_WNM_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT) */

        /* add Ralink proprietary IE to inform AP this STA is going to use AGGREGATION or PIGGY-BACK+AGGREGATION */
        /* Case I: (Aggregation + Piggy-Back) */
        /* 1. user enable aggregation, AND */
        /* 2. Mac support piggy-back */
        /* 3. AP annouces it's PIGGY-BACK+AGGREGATION-capable in BEACON */
        /* Case II: (Aggregation) */
        /* 1. user enable aggregation, AND */
        /* 2. AP annouces it's AGGREGATION-capable in BEACON */
        if(pAd->CommonCfg.bAggregationCapable)
        {
            if((pAd->CommonCfg.bPiggyBackCapable)
                    && ((pAd->MlmeAux.APRalinkIe & 0x00000003) == 3))
            {
                ULONG TmpLen;
                UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x03, 0x00, 0x00, 0x00};
                MakeOutgoingFrame(pOutBuffer + FrameLen,
                                  &TmpLen, 9, RalinkIe,
                                  END_OF_ARGS);
                FrameLen += TmpLen;
            }
            else if(pAd->MlmeAux.APRalinkIe & 0x00000001)
            {
                ULONG TmpLen;
                UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x01, 0x00, 0x00, 0x00};
                MakeOutgoingFrame(pOutBuffer + FrameLen,
                                  &TmpLen, 9, RalinkIe,
                                  END_OF_ARGS);
                FrameLen += TmpLen;
            }
        }
        else
        {
            ULONG TmpLen;
            UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x06, 0x00, 0x00, 0x00};
            MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen, 9,
                              RalinkIe, END_OF_ARGS);
            FrameLen += TmpLen;
        }

        if(pAd->MlmeAux.APEdcaParm.bValid)
        {
            if(pAd->StaCfg.UapsdInfo.bAPSDCapable
                    && pAd->MlmeAux.APEdcaParm.bAPSDCapable)
            {
                QBSS_STA_INFO_PARM QosInfo;

                NdisZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));
                QosInfo.UAPSD_AC_BE = pAd->CommonCfg.bAPSDAC_BE;
                QosInfo.UAPSD_AC_BK = pAd->CommonCfg.bAPSDAC_BK;
                QosInfo.UAPSD_AC_VI = pAd->CommonCfg.bAPSDAC_VI;
                QosInfo.UAPSD_AC_VO = pAd->CommonCfg.bAPSDAC_VO;
                QosInfo.MaxSPLength = pAd->CommonCfg.MaxSPLength;
                WmeIe[8] |= *(PUCHAR) & QosInfo;
            }
            else
            {
                /* The Parameter Set Count is set to \A1\A70\A1\A8 in the association request frames */
                /* WmeIe[8] |= (pAd->MlmeAux.APEdcaParm.EdcaUpdateCount & 0x0f); */
            }

            MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
                              9, &WmeIe[0], END_OF_ARGS);
            FrameLen += tmp;
        }

        /* */
        /* Let WPA(#221) Element ID on the end of this association frame. */
        /* Otherwise some AP will fail on parsing Element ID and set status fail on Assoc Rsp. */
        /* For example: Put Vendor Specific IE on the front of WPA IE. */
        /* This happens on AP (Model No:Linksys WRK54G) */
        /* */
        if(((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK) ||
                (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK) ||
                (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) ||
                (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2)
           )
          )
        {
            UCHAR RSNIe = IE_WPA;

            if((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)
                    || (pAd->StaCfg.AuthMode ==
                        Ndis802_11AuthModeWPA2))
            {
                RSNIe = IE_WPA2;
            }

#ifdef WPA_SUPPLICANT_SUPPORT

            if(pAd->StaCfg.bRSN_IE_FromWpaSupplicant == FALSE)
#endif /* WPA_SUPPLICANT_SUPPORT */
            {
                RTMPMakeRSNIE(pAd, pAd->StaCfg.AuthMode,
                              pAd->StaCfg.WepStatus, BSS0);

                /* Check for WPA PMK cache list */
                if(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2)
                {
                    INT idx;
                    BOOLEAN FoundPMK = FALSE;

                    /* Search chched PMKID, append it if existed */
                    for(idx = 0; idx < PMKID_NO; idx++)
                    {
                        if(NdisEqualMemory(ApAddr, &pAd->StaCfg.SavedPMK[idx].BSSID, 6))
                        {
                            FoundPMK = TRUE;
                            break;
                        }
                    }

#ifdef WPA_SUPPLICANT_SUPPORT

                    /*
                       When AuthMode is WPA2-Enterprise and AP reboot or STA lost AP,
                       AP would not do PMK cache with STA after STA re-connect to AP again.
                       In this case, driver doesn't need to send PMKID to AP and WpaSupplicant.
                     */
                    if((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2)
                            && (NdisEqualMemory(pAd->MlmeAux.Bssid, pAd->CommonCfg.LastBssid, MAC_ADDR_LEN)))
                    {
                        FoundPMK = FALSE;
                    }

#endif /* WPA_SUPPLICANT_SUPPORT */

                    if(FoundPMK)
                    {
                        /* Set PMK number */
                        *(PUSHORT) & pAd->StaCfg.RSN_IE[pAd->StaCfg.RSNIE_Len] = 1;
                        NdisMoveMemory(&pAd->StaCfg.RSN_IE[pAd->StaCfg.RSNIE_Len + 2],
                                       &pAd->StaCfg.SavedPMK[idx].PMKID, 16);
                        pAd->StaCfg.RSNIE_Len += 18;
                    }
                }
            }

#ifdef WPA_SUPPLICANT_SUPPORT

            /*
            	Can not use SIOCSIWGENIE definition, it is used in wireless.h
            	We will not see the definition in MODULE.
            	The definition can be saw in UTIL and NETIF.
            */
            /* #ifdef SIOCSIWGENIE */
            if((pAd->StaCfg.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE)
                    && (pAd->StaCfg.bRSN_IE_FromWpaSupplicant == TRUE))
            {
                ;
            }
            else
                /* #endif */
#endif /* WPA_SUPPLICANT_SUPPORT */
            {
                MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
                                  1, &RSNIe,
                                  1, &pAd->StaCfg.RSNIE_Len,
                                  pAd->StaCfg.RSNIE_Len,
                                  pAd->StaCfg.RSN_IE,
                                  END_OF_ARGS);
                FrameLen += tmp;
            }

#ifdef WPA_SUPPLICANT_SUPPORT

            /*
            	Can not use SIOCSIWGENIE definition, it is used in wireless.h
            	We will not see the definition in MODULE.
            	The definition can be saw in UTIL and NETIF.
            */
            /* #ifdef SIOCSIWGENIE */
            if(((pAd->StaCfg.WpaSupplicantUP & 0x7F) !=
                    WPA_SUPPLICANT_ENABLE)
                    || (pAd->StaCfg.bRSN_IE_FromWpaSupplicant == FALSE))
                /* #endif */
#endif /* WPA_SUPPLICANT_SUPPORT */
            {
                /* Append Variable IE */
                NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, &RSNIe, 1);
                VarIesOffset += 1;
                NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, &pAd->StaCfg.RSNIE_Len, 1);
                VarIesOffset += 1;

                NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, pAd->StaCfg.RSN_IE, pAd->StaCfg.RSNIE_Len);
                VarIesOffset += pAd->StaCfg.RSNIE_Len;

                /* Set Variable IEs Length */
                pAd->StaCfg.ReqVarIELen = VarIesOffset;
            }
        }

#ifdef WPA_SUPPLICANT_SUPPORT
        DBGPRINT(RT_DEBUG_TRACE, ("wpa_supplicant support enabled\n"));

        /*
        	Can not use SIOCSIWGENIE definition, it is used in wireless.h
        	We will not see the definition in MODULE.
        	The definition can be saw in UTIL and NETIF.
        */
        /* #ifdef SIOCSIWGENIE */
        if((pAd->StaCfg.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE) &&
                (pAd->StaCfg.bRSN_IE_FromWpaSupplicant == TRUE))
        {
            ULONG TmpWpaAssocIeLen = 0;
            MakeOutgoingFrame(pOutBuffer + FrameLen,
                              &TmpWpaAssocIeLen,
                              pAd->StaCfg.WpaAssocIeLen,
                              pAd->StaCfg.pWpaAssocIe, END_OF_ARGS);

            FrameLen += TmpWpaAssocIeLen;

            NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset,
                           pAd->StaCfg.pWpaAssocIe,
                           pAd->StaCfg.WpaAssocIeLen);
            VarIesOffset += pAd->StaCfg.WpaAssocIeLen;

            /* Set Variable IEs Length */
            pAd->StaCfg.ReqVarIELen = VarIesOffset;
        }

        /* #endif */
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef WFD_SUPPORT
#ifdef RT_CFG80211_SUPPORT

        if(pAd->StaCfg.WfdCfg.bSuppInsertWfdIe)
        {
            ULONG	WfdIeLen, WfdIeBitmap;
            PUCHAR	ptr;

            ptr = pOutBuffer + FrameLen;
            WfdIeBitmap = (0x1 << SUBID_WFD_DEVICE_INFO) | (0x1 << SUBID_WFD_ASSOCIATED_BSSID) |
                          (0x1 << SUBID_WFD_COUPLED_SINK_INFO);
            WfdMakeWfdIE(pAd, WfdIeBitmap, ptr, &WfdIeLen);
            FrameLen += WfdIeLen;
        }

#endif /* RT_CFG80211_SUPPORT */
#endif /* WFD_SUPPORT */

        MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
        MlmeFreeMemory(pAd, pOutBuffer);

        RTMPSetTimer(&pAd->MlmeAux.AssocTimer, Timeout);
        pAd->Mlme.AssocMachine.CurrState = ASSOC_WAIT_RSP;
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE,
                 ("ASSOC - MlmeAssocReqAction() sanity check failed. BUG!!!!!!\n"));
        pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
        Status = MLME_INVALID_FORMAT;
        MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2,
                    &Status, 0);
    }

}

/*
	==========================================================================
	Description:
		mlme reassoc req handling procedure
	Parameters:
		Elem -
	Pre:
		-# SSID  (Adapter->StaCfg.ssid[])
		-# BSSID (AP address, Adapter->StaCfg.bssid)
		-# Supported rates (Adapter->StaCfg.supported_rates[])
		-# Supported rates length (Adapter->StaCfg.supported_rates_len)
		-# Tx power (Adapter->StaCfg.tx_power)

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MlmeReassocReqAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    UCHAR ApAddr[6];
    HEADER_802_11 ReassocHdr;
    UCHAR WmeIe[9] = {IE_VENDOR_SPECIFIC, 0x07, 0x00, 0x50, 0xf2, 0x02, 0x00, 0x01, 0x00};
    USHORT CapabilityInfo, ListenIntv;
    ULONG Timeout;
    ULONG FrameLen = 0;
    BOOLEAN TimerCancelled;
    NDIS_STATUS NStatus;
    ULONG tmp;
    PUCHAR pOutBuffer = NULL;
    USHORT Status;

#ifdef WPA_SUPPLICANT_SUPPORT
    DBGPRINT(RT_DEBUG_TRACE, ("wpa_supplicant support enabled\n"));
#endif

#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
    DBGPRINT(RT_DEBUG_TRACE, ("Native wpa_supplicant support enabled\n"));
#endif

    /* Block all authentication request durning WPA block period */
    if(pAd->StaCfg.bBlockAssoc == TRUE)
    {
        DBGPRINT(RT_DEBUG_TRACE,
                 ("ASSOC - Block ReAssoc request durning WPA block period!\n"));
        pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
        Status = MLME_STATE_MACHINE_REJECT;
        MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_REASSOC_CONF, 2, &Status, 0);
    }
    /* the parameters are the same as the association */
    else if(MlmeAssocReqSanity(pAd, Elem->Msg, Elem->MsgLen, ApAddr, &CapabilityInfo, &Timeout, &ListenIntv))
    {
        /*for dhcp,issue ,wpa_supplicant ioctl too fast , at link_up, it will add key before driver remove key  */
        RTMPWPARemoveAllKeys(pAd);

        RTMPCancelTimer(&pAd->MlmeAux.ReassocTimer, &TimerCancelled);

        NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	/*Get an unused nonpaged memory */

        if(NStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_TRACE,
                     ("ASSOC - MlmeReassocReqAction() allocate memory failed\n"));
            pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
            Status = MLME_FAIL_NO_RESOURCE;
            MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE,
                        MT2_REASSOC_CONF, 2, &Status, 0);
            return;
        }

        COPY_MAC_ADDR(pAd->MlmeAux.Bssid, ApAddr);

        /* make frame, use bssid as the AP address?? */
        DBGPRINT(RT_DEBUG_ERROR,
                 ("ASSOC - Send RE-ASSOC request...\n"));
        MgtMacHeaderInit(pAd, &ReassocHdr, SUBTYPE_REASSOC_REQ, 0, ApAddr,
                         ApAddr);
        MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11),
                          &ReassocHdr, 2, &CapabilityInfo, 2,
                          &ListenIntv, MAC_ADDR_LEN, ApAddr, 1, &SsidIe,
                          1, &pAd->MlmeAux.SsidLen,
                          pAd->MlmeAux.SsidLen, pAd->MlmeAux.Ssid, 1,
                          &SupRateIe, 1, &pAd->MlmeAux.SupRateLen,
                          pAd->MlmeAux.SupRateLen, pAd->MlmeAux.SupRate,
                          END_OF_ARGS);

        if(pAd->MlmeAux.ExtRateLen != 0)
        {
            MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
                              1, &ExtRateIe,
                              1, &pAd->MlmeAux.ExtRateLen,
                              pAd->MlmeAux.ExtRateLen,
                              pAd->MlmeAux.ExtRate, END_OF_ARGS);
            FrameLen += tmp;
        }

        if(pAd->MlmeAux.APEdcaParm.bValid)
        {
            if(pAd->StaCfg.UapsdInfo.bAPSDCapable
                    && pAd->MlmeAux.APEdcaParm.bAPSDCapable)
            {
                QBSS_STA_INFO_PARM QosInfo;

                NdisZeroMemory(&QosInfo,
                               sizeof(QBSS_STA_INFO_PARM));
                QosInfo.UAPSD_AC_BE = pAd->CommonCfg.bAPSDAC_BE;
                QosInfo.UAPSD_AC_BK = pAd->CommonCfg.bAPSDAC_BK;
                QosInfo.UAPSD_AC_VI = pAd->CommonCfg.bAPSDAC_VI;
                QosInfo.UAPSD_AC_VO = pAd->CommonCfg.bAPSDAC_VO;
                QosInfo.MaxSPLength =
                    pAd->CommonCfg.MaxSPLength;

                DBGPRINT(RT_DEBUG_TRACE,
                         ("uapsd> MaxSPLength = %d!\n",
                          QosInfo.MaxSPLength));
                WmeIe[8] |= *(PUCHAR) & QosInfo;
            }

            MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
                              9, &WmeIe[0], END_OF_ARGS);
            FrameLen += tmp;
        }

#ifdef DOT11_N_SUPPORT

        /* HT */
        if((pAd->MlmeAux.HtCapabilityLen > 0)
                && WMODE_CAP_N(pAd->CommonCfg.PhyMode))
        {
            ULONG TmpLen;
            UCHAR HtLen;
            UCHAR BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
            PHT_CAPABILITY_IE pHtCapability;

#ifdef RT_BIG_ENDIAN
            HT_CAPABILITY_IE HtCapabilityTmp;
            NdisZeroMemory(&HtCapabilityTmp, sizeof(HT_CAPABILITY_IE));
            NdisMoveMemory(&HtCapabilityTmp, &pAd->MlmeAux.HtCapability, pAd->MlmeAux.HtCapabilityLen);
            *(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
            *(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
            pHtCapability = &HtCapabilityTmp;
#else
            pHtCapability = &pAd->MlmeAux.HtCapability;
#endif

            if(pAd->StaActive.SupportedPhyInfo.bPreNHt == TRUE)
            {
                HtLen = SIZE_HT_CAP_IE + 4;
                MakeOutgoingFrame(pOutBuffer + FrameLen,
                                  &TmpLen, 1, &WpaIe, 1, &HtLen,
                                  4, &BROADCOM[0],
                                  pAd->MlmeAux.HtCapabilityLen,
                                  pHtCapability, END_OF_ARGS);
            }
            else
            {
                MakeOutgoingFrame(pOutBuffer + FrameLen,
                                  &TmpLen, 1, &HtCapIe, 1,
                                  &pAd->MlmeAux.HtCapabilityLen,
                                  pAd->MlmeAux.HtCapabilityLen,
                                  pHtCapability, END_OF_ARGS);
            }

            FrameLen += TmpLen;

#ifdef DOT11_VHT_AC

            if(WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
                    (pAd->MlmeAux.Channel > 14) &&
                    (pAd->MlmeAux.vht_cap_len)
              )
            {
                FrameLen += build_vht_ies(pAd, (UCHAR *)(pOutBuffer + FrameLen), SUBTYPE_ASSOC_REQ);
            }

#endif /* DOT11_VHT_AC */
        }

#endif /* DOT11_N_SUPPORT */

        if(FALSE
          )
        {
            ULONG TmpLen;
            EXT_CAP_INFO_ELEMENT extCapInfo;
            UCHAR extInfoLen;

            NdisZeroMemory(&extCapInfo, sizeof(EXT_CAP_INFO_ELEMENT));
            extInfoLen = sizeof(EXT_CAP_INFO_ELEMENT);



            MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
                              1,					&ExtCapIe,
                              1					&extInfoLen,
                              extInfoLen,			&extCapInfo,
                              END_OF_ARGS);
            FrameLen += TmpLen;
            /*printk("iverson test extCapInfo.BssTransitionManmt %x\n",extCapInfo.BssTransitionManmt); */
        }

        /* add Ralink proprietary IE to inform AP this STA is going to use AGGREGATION or PIGGY-BACK+AGGREGATION */
        /* Case I: (Aggregation + Piggy-Back) */
        /* 1. user enable aggregation, AND */
        /* 2. Mac support piggy-back */
        /* 3. AP annouces it's PIGGY-BACK+AGGREGATION-capable in BEACON */
        /* Case II: (Aggregation) */
        /* 1. user enable aggregation, AND */
        /* 2. AP annouces it's AGGREGATION-capable in BEACON */
        if(pAd->CommonCfg.bAggregationCapable)
        {
            if((pAd->CommonCfg.bPiggyBackCapable)
                    && ((pAd->MlmeAux.APRalinkIe & 0x00000003) == 3))
            {
                ULONG TmpLen;
                UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x03, 0x00, 0x00, 0x00 };
                MakeOutgoingFrame(pOutBuffer + FrameLen,
                                  &TmpLen, 9, RalinkIe,
                                  END_OF_ARGS);
                FrameLen += TmpLen;
            }
            else if(pAd->MlmeAux.APRalinkIe & 0x00000001)
            {
                ULONG TmpLen;
                UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x01, 0x00, 0x00, 0x00 };
                MakeOutgoingFrame(pOutBuffer + FrameLen,
                                  &TmpLen, 9, RalinkIe,
                                  END_OF_ARGS);
                FrameLen += TmpLen;
            }
        }
        else
        {
            ULONG TmpLen;
            UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x04, 0x00, 0x00, 0x00 };
            MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen, 9,
                              RalinkIe, END_OF_ARGS);
            FrameLen += TmpLen;
        }

#ifdef WFD_SUPPORT
#ifdef RT_CFG80211_SUPPORT

        if(pAd->StaCfg.WfdCfg.bSuppInsertWfdIe)
        {
            ULONG	WfdIeLen, WfdIeBitmap;
            PUCHAR	ptr;

            ptr = pOutBuffer + FrameLen;
            WfdIeBitmap = (0x1 << SUBID_WFD_DEVICE_INFO) | (0x1 << SUBID_WFD_ASSOCIATED_BSSID) |
                          (0x1 << SUBID_WFD_COUPLED_SINK_INFO);
            WfdMakeWfdIE(pAd, WfdIeBitmap, ptr, &WfdIeLen);
            FrameLen += WfdIeLen;
        }

#endif /* RT_CFG80211_SUPPORT */
#endif /* WFD_SUPPORT */

        MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
        MlmeFreeMemory(pAd, pOutBuffer);

        RTMPSetTimer(&pAd->MlmeAux.ReassocTimer, Timeout * 2);	/* in mSec */
        pAd->Mlme.AssocMachine.CurrState = REASSOC_WAIT_RSP;
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE,
                 ("ASSOC - MlmeReassocReqAction() sanity check failed. BUG!!!!\n"));
        pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
        Status = MLME_INVALID_FORMAT;
        MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_REASSOC_CONF, 2,
                    &Status, 0);
    }
}

/*
	==========================================================================
	Description:
		Upper layer issues disassoc request
	Parameters:
		Elem -

	IRQL = PASSIVE_LEVEL

	==========================================================================
 */
VOID MlmeDisassocReqAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    PMLME_DISASSOC_REQ_STRUCT pDisassocReq;
    HEADER_802_11 DisassocHdr;
    PHEADER_802_11 pDisassocHdr;
    PUCHAR pOutBuffer = NULL;
    ULONG FrameLen = 0;
    NDIS_STATUS NStatus;
    BOOLEAN TimerCancelled;
    ULONG Timeout = 500;
    USHORT Status;

#ifdef QOS_DLS_SUPPORT

    /* send DLS-TEAR_DOWN message, */
    if(pAd->CommonCfg.bDLSCapable)
    {
        UCHAR i;

        /* tear down local dls table entry */
        for(i = 0; i < MAX_NUM_OF_INIT_DLS_ENTRY; i++)
        {
            if(pAd->StaCfg.DLSEntry[i].Valid
                    && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH))
            {
                RTMPSendDLSTearDownFrame(pAd, pAd->StaCfg.DLSEntry[i].MacAddr);
                pAd->StaCfg.DLSEntry[i].Status = DLS_NONE;
                pAd->StaCfg.DLSEntry[i].Valid = FALSE;
            }
        }

        /* tear down peer dls table entry */
        for(i = MAX_NUM_OF_INIT_DLS_ENTRY; i < MAX_NUM_OF_DLS_ENTRY; i++)
        {
            if(pAd->StaCfg.DLSEntry[i].Valid
                    && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH))
            {
                RTMPSendDLSTearDownFrame(pAd, pAd->StaCfg.DLSEntry[i].MacAddr);
                pAd->StaCfg.DLSEntry[i].Status = DLS_NONE;
                pAd->StaCfg.DLSEntry[i].Valid = FALSE;
            }
        }
    }

#endif /* QOS_DLS_SUPPORT */


    /* skip sanity check */
    pDisassocReq = (PMLME_DISASSOC_REQ_STRUCT)(Elem->Msg);

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	/*Get an unused nonpaged memory */

    if(NStatus != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE,
                 ("ASSOC - MlmeDisassocReqAction() allocate memory failed\n"));
        pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
        Status = MLME_FAIL_NO_RESOURCE;
        MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_DISASSOC_CONF, 2,
                    &Status, 0);
        return;
    }


    RTMPCancelTimer(&pAd->MlmeAux.DisassocTimer, &TimerCancelled);

    DBGPRINT(RT_DEBUG_ERROR,
             ("ASSOC - Send DISASSOC request[BSSID::%02x:%02x:%02x:%02x:%02x:%02x (Reason=%d)\n",
              PRINT_MAC(pDisassocReq->Addr), pDisassocReq->Reason));
    MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pDisassocReq->Addr,
                     pDisassocReq->Addr);	/* patch peap ttls switching issue */
    MakeOutgoingFrame(pOutBuffer, &FrameLen,
                      sizeof(HEADER_802_11), &DisassocHdr,
                      2, &pDisassocReq->Reason, END_OF_ARGS);
    MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);

    /* To patch Instance and Buffalo(N) AP */
    /* Driver has to send deauth to Instance AP, but Buffalo(N) needs to send disassoc to reset Authenticator's state machine */
    /* Therefore, we send both of them. */
    pDisassocHdr = (PHEADER_802_11) pOutBuffer;
    pDisassocHdr->FC.SubType = SUBTYPE_DEAUTH;
    MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);

    MlmeFreeMemory(pAd, pOutBuffer);

    pAd->StaCfg.DisassocReason = REASON_DISASSOC_STA_LEAVING;
    COPY_MAC_ADDR(pAd->StaCfg.DisassocSta, pDisassocReq->Addr);

    RTMPSetTimer(&pAd->MlmeAux.DisassocTimer, Timeout);	/* in mSec */
    pAd->Mlme.AssocMachine.CurrState = DISASSOC_WAIT_RSP;

#ifdef WPA_SUPPLICANT_SUPPORT

    DBGPRINT(RT_DEBUG_ERROR, ("wpa_supplicant support enabled\n"));


#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT

    if(pAd->StaCfg.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE)
    {
        /*send disassociate event to wpa_supplicant */
        RtmpOSWirelessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM,
                                RT_DISASSOC_EVENT_FLAG, NULL, NULL, 0);
    }

#else

    DBGPRINT(RT_DEBUG_TRACE, ("Native wpa_supplicant support enabled\n"));

#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */


    /* mark here because linkdown also call this function */

    RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, NULL, BSS0, 0);

}

/*
	==========================================================================
	Description:
		peer sends assoc rsp back
	Parameters:
		Elme - MLME message containing the received frame

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID PeerAssocRspAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    USHORT CapabilityInfo, Status, Aid;
    UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES], SupRateLen;
    UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRateLen;
    UCHAR Addr2[MAC_ADDR_LEN];
    BOOLEAN TimerCancelled;
    UCHAR CkipFlag;
    EDCA_PARM EdcaParm;
    HT_CAPABILITY_IE HtCapability;
    ADD_HT_INFO_IE AddHtInfo;	/* AP might use this additional ht info IE */
    UCHAR HtCapabilityLen = 0;
    UCHAR AddHtInfoLen;
    UCHAR NewExtChannelOffset = 0xff;
    EXT_CAP_INFO_ELEMENT ExtCapInfo;
    MAC_TABLE_ENTRY *pEntry;
    IE_LISTS *ie_list = NULL;


    os_alloc_mem(pAd, (UCHAR **)&ie_list, sizeof(IE_LISTS));

    if(ie_list == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s(): mem alloc failed!\n", __FUNCTION__));
        return;
    }

    NdisZeroMemory((UCHAR *)ie_list, sizeof(IE_LISTS));

    if(PeerAssocRspSanity(pAd, Elem->Msg, Elem->MsgLen,
                          Addr2, &CapabilityInfo, &Status, &Aid, SupRate,
                          &SupRateLen, ExtRate, &ExtRateLen, &HtCapability,
                          &AddHtInfo, &HtCapabilityLen, &AddHtInfoLen,
                          &NewExtChannelOffset, &EdcaParm, &ExtCapInfo,
                          &CkipFlag, ie_list))
    {
        /* The frame is for me ? */
        if(MAC_ADDR_EQUAL(Addr2, pAd->MlmeAux.Bssid))
        {
            DBGPRINT(RT_DEBUG_TRACE,
                     ("%s():ASSOC - receive ASSOC_RSP to me (status=%d)\n", __FUNCTION__, Status));
#ifdef DOT11_N_SUPPORT
            DBGPRINT(RT_DEBUG_TRACE,
                     ("%s():MacTable [%d].AMsduSize = %d. ClientStatusFlags = 0x%lx\n",
                      __FUNCTION__, Elem->Wcid,
                      pAd->MacTab.Content[BSSID_WCID].AMsduSize,
                      pAd->MacTab.Content[BSSID_WCID].ClientStatusFlags));
#endif /* DOT11_N_SUPPORT */
            RTMPCancelTimer(&pAd->MlmeAux.AssocTimer, &TimerCancelled);



            if(Status == MLME_SUCCESS)
            {
                UCHAR MaxSupportedRateIn500Kbps = 0;

                /*
                	In roaming case, LinkDown wouldn't be invoked.
                	For preventing finding MacTable Hash index malfunction,
                	we need to do MacTableDeleteEntry here.
                */
                pEntry = MacTableLookup(pAd, pAd->CommonCfg.Bssid);

                if(pEntry)
                {
                    MacTableDeleteEntry(pAd, pEntry->Aid, pEntry->Addr);
                    pEntry = NULL;
                }

                MaxSupportedRateIn500Kbps = dot11_max_sup_rate(SupRateLen, &SupRate[0],
                                            ExtRateLen, &ExtRate[0]);
                /* go to procedure listed on page 376 */
                AssocPostProc(pAd, Addr2, CapabilityInfo, Aid,
                              SupRate, SupRateLen, ExtRate,
                              ExtRateLen, &EdcaParm,
                              ie_list,
                              &HtCapability, HtCapabilityLen,
                              &AddHtInfo);

                StaAddMacTableEntry(pAd,
                                    &pAd->MacTab.Content[BSSID_WCID],
                                    MaxSupportedRateIn500Kbps,
                                    &HtCapability,
                                    HtCapabilityLen, &AddHtInfo,
                                    AddHtInfoLen,
                                    ie_list,
                                    CapabilityInfo);

                RTMPSetSupportMCS(pAd,
                                  OPMODE_STA,
                                  &pAd->MacTab.Content[BSSID_WCID],
                                  SupRate,
                                  SupRateLen,
                                  ExtRate,
                                  ExtRateLen,
#ifdef DOT11_VHT_AC
                                  ie_list->vht_cap_len,
                                  &ie_list->vht_cap,
#endif /* DOT11_VHT_AC */
                                  &HtCapability,
                                  HtCapabilityLen);

            }

            pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
            MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2, &Status, 0);

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
            {
                /*PFRAME_802_11 pFrame = (PFRAME_802_11) (Elem->Msg);
                RTEnqueueInternalCmd(pAd,
                		     CMDTHREAD_CONNECT_RESULT_INFORM,
                		     &pFrame->Octet[6],
                		     Elem->MsgLen - 6 - sizeof (HEADER_802_11));*/

                PFRAME_802_11 pFrame = (PFRAME_802_11)(Elem->Msg);
                // XTKNIGHT_FIX: this is what makes WPA2 work with supplicant on nl80211
                DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - %s() connect inform\n", __FUNCTION__));

                RT_CFG80211_CONN_RESULT_INFORM(pAd, pAd->MlmeAux.Bssid,
                                               pAd->StaCfg.ReqVarIEs, pAd->StaCfg.ReqVarIELen,
                                               &pFrame->Octet[6],
                                               Elem->MsgLen - 6 - sizeof(HEADER_802_11),
                                               TRUE);

            }
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - %s() sanity check fail\n", __FUNCTION__));
    }

    if(ie_list != NULL)
        os_free_mem(NULL, ie_list);
}

/*
	==========================================================================
	Description:
		peer sends reassoc rsp
	Parametrs:
		Elem - MLME message cntaining the received frame

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID PeerReassocRspAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    USHORT CapabilityInfo;
    USHORT Status;
    USHORT Aid;
    UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES], SupRateLen;
    UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRateLen;
    UCHAR Addr2[MAC_ADDR_LEN];
    UCHAR CkipFlag;
    BOOLEAN TimerCancelled;
    EDCA_PARM EdcaParm;
    HT_CAPABILITY_IE HtCapability;
    ADD_HT_INFO_IE AddHtInfo;	/* AP might use this additional ht info IE */
    UCHAR HtCapabilityLen;
    UCHAR AddHtInfoLen;
    UCHAR NewExtChannelOffset = 0xff;
    EXT_CAP_INFO_ELEMENT ExtCapInfo;
    IE_LISTS *ie_list = NULL;

    os_alloc_mem(pAd, (UCHAR **)&ie_list, sizeof(IE_LISTS));

    if(ie_list == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s(): mem alloc failed!\n", __FUNCTION__));
        return;
    }

    NdisZeroMemory((UCHAR *)ie_list, sizeof(IE_LISTS));

    if(PeerAssocRspSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2,
                          &CapabilityInfo, &Status, &Aid, SupRate,
                          &SupRateLen, ExtRate, &ExtRateLen, &HtCapability,
                          &AddHtInfo, &HtCapabilityLen, &AddHtInfoLen,
                          &NewExtChannelOffset, &EdcaParm, &ExtCapInfo,
                          &CkipFlag, ie_list))
    {
        if(MAC_ADDR_EQUAL(Addr2, pAd->MlmeAux.Bssid))  	/* The frame is for me ? */
        {
            DBGPRINT(RT_DEBUG_ERROR,
                     ("REASSOC - receive REASSOC_RSP to me (status=%d)\n", Status));
            RTMPCancelTimer(&pAd->MlmeAux.ReassocTimer,
                            &TimerCancelled);

            if(Status == MLME_SUCCESS)
            {
                UCHAR MaxSupportedRateIn500Kbps = 0;
                PMAC_TABLE_ENTRY pEntry = NULL;

                /*
                	In roaming case, LinkDown wouldn't be invoked.
                	For preventing finding MacTable Hash index malfunction,
                	we need to do MacTableDeleteEntry here.
                */
                pEntry = MacTableLookup(pAd, pAd->CommonCfg.Bssid);

                if(pEntry)
                {
                    MacTableDeleteEntry(pAd, pEntry->Aid, pEntry->Addr);
                    pEntry = NULL;
                }

                MaxSupportedRateIn500Kbps = dot11_max_sup_rate(SupRateLen, &SupRate[0], ExtRateLen, &ExtRate[0]);


                /* go to procedure listed on page 376 */
                AssocPostProc(pAd, Addr2, CapabilityInfo, Aid,
                              SupRate, SupRateLen, ExtRate,
                              ExtRateLen, &EdcaParm,
                              ie_list,
                              &HtCapability, HtCapabilityLen,
                              &AddHtInfo);

                StaAddMacTableEntry(pAd,
                                    &pAd->MacTab.Content[BSSID_WCID],
                                    MaxSupportedRateIn500Kbps,
                                    &HtCapability,
                                    HtCapabilityLen, &AddHtInfo,
                                    AddHtInfoLen,
                                    ie_list,
                                    CapabilityInfo);

                RTMPSetSupportMCS(pAd,
                                  OPMODE_STA,
                                  &pAd->MacTab.Content[BSSID_WCID],
                                  SupRate,
                                  SupRateLen,
                                  ExtRate,
                                  ExtRateLen,
#ifdef DOT11_VHT_AC
                                  ie_list->vht_cap_len,
                                  &ie_list->vht_cap,
#endif /* DOT11_VHT_AC */
                                  &HtCapability,
                                  HtCapabilityLen);

#ifdef WPA_SUPPLICANT_SUPPORT
                DBGPRINT(RT_DEBUG_TRACE, ("wpa_supplicant support enabled"));
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT

                if(pAd->StaCfg.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE)
                {
                    SendAssocIEsToWpaSupplicant(pAd->net_dev,
                                                pAd->StaCfg.ReqVarIEs,
                                                pAd->StaCfg.ReqVarIELen);
                    RtmpOSWirelessEventSend(pAd->net_dev,
                                            RT_WLAN_EVENT_CUSTOM,
                                            RT_ASSOC_EVENT_FLAG,
                                            NULL, NULL, 0);
                }

#else
                DBGPRINT(RT_DEBUG_TRACE, ("Native wpa_supplicant support enabled"));
#endif /* !NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
                {

                    DBGPRINT(RT_DEBUG_TRACE, ("Native wpa_supplicant support enabled (ReqVarIELen=%d)\n", pAd->StaCfg.ReqVarIELen));

                    wext_notify_event_assoc(pAd->net_dev,
                                            pAd->StaCfg.ReqVarIEs,
                                            pAd->StaCfg.ReqVarIELen);
                    RtmpOSWirelessEventSend(pAd->net_dev,
                                            RT_WLAN_EVENT_CGIWAP,
                                            -1,
                                            &pAd->MlmeAux.Bssid[0], NULL,
                                            0);
                }
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */

            }

            /* CkipFlag is no use for reassociate */
            pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
            MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_REASSOC_CONF, 2, &Status, 0);
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE,
                 ("REASSOC - %s() sanity check fail\n", __FUNCTION__));
    }

    if(ie_list)
        os_free_mem(pAd, ie_list);

}

/*
	==========================================================================
	Description:
		procedures on IEEE 802.11/1999 p.376
	Parametrs:

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AssocPostProc(
    IN PRTMP_ADAPTER pAd,
    IN PUCHAR pAddr2,
    IN USHORT CapabilityInfo,
    IN USHORT Aid,
    IN UCHAR SupRate[],
    IN UCHAR SupRateLen,
    IN UCHAR ExtRate[],
    IN UCHAR ExtRateLen,
    IN PEDCA_PARM pEdcaParm,
    IN IE_LISTS *ie_list,
    IN HT_CAPABILITY_IE *pHtCapability,
    IN UCHAR HtCapabilityLen,
    IN ADD_HT_INFO_IE *pAddHtInfo)
{
    /* AP might use this additional ht info IE */
    ULONG Idx;

    pAd->MlmeAux.BssType = BSS_INFRA;
    COPY_MAC_ADDR(pAd->MlmeAux.Bssid, pAddr2);
    pAd->MlmeAux.Aid = Aid;
    pAd->MlmeAux.CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;

#ifdef DOT11_N_SUPPORT

    /* Some HT AP might lost WMM IE. We add WMM ourselves. beacuase HT requires QoS on. */
    if((HtCapabilityLen > 0) && (pEdcaParm->bValid == FALSE))
    {
        pEdcaParm->bValid = TRUE;
        pEdcaParm->Aifsn[0] = 3;
        pEdcaParm->Aifsn[1] = 7;
        pEdcaParm->Aifsn[2] = 2;
        pEdcaParm->Aifsn[3] = 2;

        pEdcaParm->Cwmin[0] = 4;
        pEdcaParm->Cwmin[1] = 4;
        pEdcaParm->Cwmin[2] = 3;
        pEdcaParm->Cwmin[3] = 2;

        pEdcaParm->Cwmax[0] = 10;
        pEdcaParm->Cwmax[1] = 10;
        pEdcaParm->Cwmax[2] = 4;
        pEdcaParm->Cwmax[3] = 3;

        pEdcaParm->Txop[0] = 0;
        pEdcaParm->Txop[1] = 0;
        pEdcaParm->Txop[2] = 96;
        pEdcaParm->Txop[3] = 48;

    }

#endif /* DOT11_N_SUPPORT */

    NdisMoveMemory(&pAd->MlmeAux.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));

    /* filter out un-supported rates */
    pAd->MlmeAux.SupRateLen = SupRateLen;
    NdisMoveMemory(pAd->MlmeAux.SupRate, SupRate, SupRateLen);
    RTMPCheckRates(pAd, pAd->MlmeAux.SupRate, &pAd->MlmeAux.SupRateLen);

    /* filter out un-supported rates */
    pAd->MlmeAux.ExtRateLen = ExtRateLen;
    NdisMoveMemory(pAd->MlmeAux.ExtRate, ExtRate, ExtRateLen);
    RTMPCheckRates(pAd, pAd->MlmeAux.ExtRate, &pAd->MlmeAux.ExtRateLen);

#ifdef DOT11_N_SUPPORT

    if(HtCapabilityLen > 0)
    {
        RTMPCheckHt(pAd, BSSID_WCID, pHtCapability, pAddHtInfo);
    }

    DBGPRINT(RT_DEBUG_TRACE,
             ("%s():=>AP.AMsduSize = %d. ClientStatusFlags = 0x%lx\n",
              __FUNCTION__,
              pAd->MacTab.Content[BSSID_WCID].AMsduSize,
              pAd->MacTab.Content[BSSID_WCID].ClientStatusFlags));

    DBGPRINT(RT_DEBUG_TRACE,
             ("%s():=>(Mmps=%d, AmsduSize=%d, )\n",
              __FUNCTION__,
              pAd->MacTab.Content[BSSID_WCID].MmpsMode,
              pAd->MacTab.Content[BSSID_WCID].AMsduSize));

#ifdef DOT11_VHT_AC

    if(ie_list->vht_cap_len > 0 && ie_list->vht_op_len > 0)
    {
        RTMPCheckVht(pAd, BSSID_WCID, &ie_list->vht_cap, &ie_list->vht_op);
    }

#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

    /* Set New WPA information */
    Idx = BssTableSearch(&pAd->ScanTab, pAddr2, pAd->MlmeAux.Channel);

    if(Idx == BSS_NOT_FOUND)
    {
        DBGPRINT_ERR(("ASSOC - Can't find BSS after receiving Assoc response\n"));
    }
    else
    {
        /* Init variable */
        pAd->MacTab.Content[BSSID_WCID].RSNIE_Len = 0;
        NdisZeroMemory(pAd->MacTab.Content[BSSID_WCID].RSN_IE, MAX_LEN_OF_RSNIE);

        DBGPRINT(RT_DEBUG_INFO,
                 ("ASSOC - AuthMode = %d\n", pAd->StaCfg.AuthMode));

        /* Store appropriate RSN_IE for WPA SM negotiation later */
        if((pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
                && (pAd->ScanTab.BssEntry[Idx].VarIELen != 0))
        {
            PUCHAR pVIE;
            USHORT len;
            PEID_STRUCT pEid;

            DBGPRINT(RT_DEBUG_INFO,
                     ("Storing RSS_IE for WPA SM negotiation later\n"));

            pVIE = pAd->ScanTab.BssEntry[Idx].VarIEs;
            len = pAd->ScanTab.BssEntry[Idx].VarIELen;

#ifdef PCIE_PS_SUPPORT
            /* Don't allow to go to sleep mode if authmode is WPA-related. */
            /*This can make Authentication process more smoothly. */
            RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP);
#endif /* PCIE_PS_SUPPORT */

            while(len > 0)
            {
                pEid = (PEID_STRUCT) pVIE;

                /* For WPA/WPAPSK */
                if((pEid->Eid == IE_WPA)
                        && (NdisEqualMemory(pEid->Octet, WPA_OUI, 4))
                        && (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA
                            || pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK))
                {
                    NdisMoveMemory(pAd->MacTab.Content[BSSID_WCID].RSN_IE, pVIE, (pEid->Len + 2));
                    pAd->MacTab.Content[BSSID_WCID].RSNIE_Len = (pEid->Len + 2);
                    DBGPRINT(RT_DEBUG_INFO,
                             ("%s(): Store RSN_IE for WPA SM negotiation\n", __FUNCTION__));
                }
                /* For WPA2/WPA2PSK */
                else if((pEid->Eid == IE_RSN)
                        && (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3))
                        && (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2
                            || pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
                {
                    NdisMoveMemory(pAd->MacTab.Content[BSSID_WCID].RSN_IE, pVIE, (pEid->Len + 2));
                    pAd->MacTab.Content[BSSID_WCID].RSNIE_Len = (pEid->Len + 2);
                    DBGPRINT(RT_DEBUG_INFO,
                             ("%s(): Store RSN_IE for WPA2 SM negotiation\n", __FUNCTION__));
                }

                pVIE += (pEid->Len + 2);
                len -= (pEid->Len + 2);
            }


        }
        else
        {

            DBGPRINT_ERR(("Not storing RSS_IE\n"));

        }

        if(pAd->MacTab.Content[BSSID_WCID].RSNIE_Len == 0)
        {
            DBGPRINT(RT_DEBUG_ERROR,
                     ("%s():=> no RSN_IE\n", __FUNCTION__));
        }
        else
        {
            hex_dump("RSN_IE",
                     pAd->MacTab.Content[BSSID_WCID].RSN_IE,
                     pAd->MacTab.Content[BSSID_WCID].RSNIE_Len);
        }
    }
}

/*
	==========================================================================
	Description:
		left part of IEEE 802.11/1999 p.374
	Parameters:
		Elem - MLME message containing the received frame

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID PeerDisassocAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    UCHAR Addr2[MAC_ADDR_LEN];
    USHORT Reason;

    DBGPRINT(RT_DEBUG_INFO, ("ASSOC - PeerDisassocAction()\n"));

    if(PeerDisassocSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Reason))
    {
        DBGPRINT(RT_DEBUG_INFO,
                 ("ASSOC - PeerDisassocAction() Reason = %d\n",
                  Reason));

        if(INFRA_ON(pAd)
                && MAC_ADDR_EQUAL(pAd->CommonCfg.Bssid, Addr2))
        {

            RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, NULL,
                                  BSS0, 0);


            /*
               It is possible that AP sends dis-assoc frame(PeerDisassocAction) to STA
               after driver enqueue MT2_MLME_DISASSOC_REQ (MlmeDisassocReqAction)
               and set CntlMachine.CurrState = CNTL_WAIT_DISASSOC.
               DisassocTimer is useless because AssocMachine.CurrState will set to ASSOC_IDLE here.
               Therefore, we need to check CntlMachine.CurrState here and enqueue MT2_DISASSOC_CONF to
               reset CntlMachine.CurrState to CNTL_IDLE state again.
             */
            if(pAd->Mlme.CntlMachine.CurrState ==
                    CNTL_WAIT_DISASSOC)
            {
                USHORT Status;
                Status = MLME_SUCCESS;
                MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE,
                            MT2_DISASSOC_CONF, 2, &Status, 0);
            }
            else
                LinkDown(pAd, TRUE);

            pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;

#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT

            DBGPRINT(RT_DEBUG_INFO,
                     ("ASSOC - Send disassoc to WpaSupplicant(1)\n"));

            if(pAd->StaCfg.WpaSupplicantUP !=
                    WPA_SUPPLICANT_DISABLE)
            {

                DBGPRINT(RT_DEBUG_INFO,
                         ("ASSOC - Send disassoc to WpaSupplicant(2)\n"));

                /*send disassociate event to wpa_supplicant */
                RtmpOSWirelessEventSend(pAd->net_dev,
                                        RT_WLAN_EVENT_CUSTOM,
                                        RT_DISASSOC_EVENT_FLAG,
                                        NULL, NULL, 0);
            }

#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */
            /* mark here because linkdown also call this function */
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE,
                 ("ASSOC - PeerDisassocAction() sanity check fail\n"));
    }

}

/*
	==========================================================================
	Description:
		what the state machine will do after assoc timeout
	Parameters:
		Elme -

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AssocTimeoutAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    USHORT Status;
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - AssocTimeoutAction\n"));
    pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
    Status = MLME_REJ_TIMEOUT;
    MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2, &Status,
                0);
}

/*
	==========================================================================
	Description:
		what the state machine will do after reassoc timeout

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID ReassocTimeoutAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    USHORT Status;
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - ReassocTimeoutAction\n"));
    pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
    Status = MLME_REJ_TIMEOUT;
    MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_REASSOC_CONF, 2, &Status,
                0);
}

/*
	==========================================================================
	Description:
		what the state machine will do after disassoc timeout

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID DisassocTimeoutAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    USHORT Status;
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - DisassocTimeoutAction\n"));
    pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
    Status = MLME_SUCCESS;
    MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_DISASSOC_CONF, 2, &Status,
                0);
}

VOID InvalidStateWhenAssoc(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    USHORT Status;
    DBGPRINT(RT_DEBUG_TRACE,
             ("ASSOC - InvalidStateWhenAssoc(state=%ld), reset ASSOC state machine\n",
              pAd->Mlme.AssocMachine.CurrState));
    pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
    Status = MLME_STATE_MACHINE_REJECT;
    MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2, &Status,
                0);
}

VOID InvalidStateWhenReassoc(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    USHORT Status;
    DBGPRINT(RT_DEBUG_TRACE,
             ("ASSOC - InvalidStateWhenReassoc(state=%ld), reset ASSOC state machine\n",
              pAd->Mlme.AssocMachine.CurrState));
    pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
    Status = MLME_STATE_MACHINE_REJECT;
    MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_REASSOC_CONF, 2, &Status,
                0);
}

VOID InvalidStateWhenDisassociate(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    USHORT Status;
    DBGPRINT(RT_DEBUG_TRACE,
             ("ASSOC - InvalidStateWhenDisassoc(state=%ld), reset ASSOC state machine\n",
              pAd->Mlme.AssocMachine.CurrState));
    pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
    Status = MLME_STATE_MACHINE_REJECT;
    MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_DISASSOC_CONF, 2, &Status,
                0);
}

/*
	==========================================================================
	Description:
		right part of IEEE 802.11/1999 page 374
	Note:
		This event should never cause ASSOC state machine perform state
		transition, and has no relationship with CNTL machine. So we separate
		this routine as a service outside of ASSOC state transition table.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID Cls3errAction(
    IN PRTMP_ADAPTER pAd,
    IN PUCHAR pAddr)
{
    HEADER_802_11 DisassocHdr;
    PHEADER_802_11 pDisassocHdr;
    PUCHAR pOutBuffer = NULL;
    ULONG FrameLen = 0;
    NDIS_STATUS NStatus;
    USHORT Reason = REASON_CLS3ERR;

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	/*Get an unused nonpaged memory */

    if(NStatus != NDIS_STATUS_SUCCESS)
        return;

    DBGPRINT(RT_DEBUG_TRACE,
             ("ASSOC - Class 3 Error, Send DISASSOC frame\n"));
    MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pAddr,
                     pAd->CommonCfg.Bssid);	/* patch peap ttls switching issue */
    MakeOutgoingFrame(pOutBuffer, &FrameLen,
                      sizeof(HEADER_802_11), &DisassocHdr,
                      2, &Reason, END_OF_ARGS);
    MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);

    /* To patch Instance and Buffalo(N) AP */
    /* Driver has to send deauth to Instance AP, but Buffalo(N) needs to send disassoc to reset Authenticator's state machine */
    /* Therefore, we send both of them. */
    pDisassocHdr = (PHEADER_802_11) pOutBuffer;
    pDisassocHdr->FC.SubType = SUBTYPE_DEAUTH;
    MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);

    MlmeFreeMemory(pAd, pOutBuffer);

    pAd->StaCfg.DisassocReason = REASON_CLS3ERR;
    COPY_MAC_ADDR(pAd->StaCfg.DisassocSta, pAddr);
}


BOOLEAN StaAddMacTableEntry(
    IN PRTMP_ADAPTER pAd,
    IN PMAC_TABLE_ENTRY pEntry,
    IN UCHAR MaxSupportedRateIn500Kbps,
    IN HT_CAPABILITY_IE *pHtCapability,
    IN UCHAR HtCapabilityLen,
    IN ADD_HT_INFO_IE *pAddHtInfo,
    IN UCHAR AddHtInfoLen,
    IN IE_LISTS *ie_list,
    IN USHORT CapabilityInfo)
{
    UCHAR MaxSupportedRate = RATE_11;
    BOOLEAN bSupportN = FALSE;
#ifdef TXBF_SUPPORT
    BOOLEAN		supportsETxBf = FALSE;
#endif

    if(!pEntry)
        return FALSE;

    if(ADHOC_ON(pAd))
        CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

    MaxSupportedRate = dot11_2_ra_rate(MaxSupportedRateIn500Kbps);

    if(WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_G)
            && (MaxSupportedRate < RATE_FIRST_OFDM_RATE))
        return FALSE;

#ifdef DOT11_N_SUPPORT

    /* 11n only */
    if(WMODE_HT_ONLY(pAd->CommonCfg.PhyMode)
            && (HtCapabilityLen == 0))
        return FALSE;

#endif /* DOT11_N_SUPPORT */

    NdisAcquireSpinLock(&pAd->MacTabLock);

    if(pEntry)
    {
        NdisZeroMemory(pEntry->R_Counter, sizeof(pEntry->R_Counter));
        pEntry->PortSecured = WPA_802_1X_PORT_SECURED;

        if((MaxSupportedRate < RATE_FIRST_OFDM_RATE) ||
                WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
        {
            pEntry->RateLen = 4;

            if(MaxSupportedRate >= RATE_FIRST_OFDM_RATE)
                MaxSupportedRate = RATE_11;
        }
        else
            pEntry->RateLen = 12;

        pEntry->MaxHTPhyMode.word = 0;
        pEntry->MinHTPhyMode.word = 0;
        pEntry->HTPhyMode.word = 0;
        pEntry->MaxSupportedRate = MaxSupportedRate;

        set_entry_phy_cfg(pAd, pEntry);

        pEntry->CapabilityInfo = CapabilityInfo;
        pEntry->isCached = FALSE;
        CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
        CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
    }

#ifdef DOT11_N_SUPPORT
    NdisZeroMemory(&pEntry->HTCapability, sizeof(pEntry->HTCapability));

    /* If this Entry supports 802.11n, upgrade to HT rate. */
    if(((pAd->StaCfg.WepStatus != Ndis802_11WEPEnabled)
            && (pAd->StaCfg.WepStatus != Ndis802_11Encryption2Enabled))
            || (pAd->CommonCfg.HT_DisallowTKIP == FALSE))
    {
        if((pAd->StaCfg.BssType == BSS_INFRA) &&
                (HtCapabilityLen != 0) &&
                WMODE_CAP_N(pAd->CommonCfg.PhyMode))
            bSupportN = TRUE;

        if((pAd->StaCfg.BssType == BSS_ADHOC) &&
                (pAd->StaCfg.bAdhocN == TRUE) &&
                (HtCapabilityLen != 0) &&
                WMODE_CAP_N(pAd->CommonCfg.PhyMode))
            bSupportN = TRUE;
    }

    if(bSupportN)
    {
        if(ADHOC_ON(pAd))
            CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

        ht_mode_adjust(pAd, pEntry, pHtCapability, &pAd->CommonCfg.DesiredHtPhy);

#ifdef TXBF_SUPPORT

        if(pAd->chipCap.FlgHwTxBfCap)
            supportsETxBf = clientSupportsETxBF(pAd, &pHtCapability->TxBFCap);

#endif /* TXBF_SUPPORT */

        /* find max fixed rate */
        pEntry->MaxHTPhyMode.field.MCS = get_ht_max_mcs(pAd, &pAd->StaCfg.DesiredHtPhyInfo.MCSSet[0], &pHtCapability->MCSSet[0]);

        if(pAd->StaCfg.DesiredTransmitSetting.field.MCS != MCS_AUTO)
            set_ht_fixed_mcs(pAd, pEntry, pAd->StaCfg.DesiredTransmitSetting.field.MCS, pAd->StaCfg.HTPhyMode.field.MCS);

        pEntry->MaxHTPhyMode.field.STBC = (pHtCapability->HtCapInfo.RxSTBC & (pAd->CommonCfg.DesiredHtPhy.TxSTBC));
        pEntry->MpduDensity = pHtCapability->HtCapParm.MpduDensity;
        pEntry->MaxRAmpduFactor = pHtCapability->HtCapParm.MaxRAmpduFactor;
        pEntry->MmpsMode = (UCHAR) pHtCapability->HtCapInfo.MimoPs;
        pEntry->AMsduSize = (UCHAR) pHtCapability->HtCapInfo.AMsduSize;
        pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;

        if(pAd->CommonCfg.DesiredHtPhy.AmsduEnable
                && (pAd->CommonCfg.REGBACapability.field.AutoBA == FALSE))
            CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_AMSDU_INUSED);

        if(pHtCapability->HtCapInfo.ShortGIfor20)
            CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE);

        if(pHtCapability->HtCapInfo.ShortGIfor40)
            CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE);

        if(pHtCapability->HtCapInfo.TxSTBC)
            CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_TxSTBC_CAPABLE);

        if(pHtCapability->HtCapInfo.RxSTBC)
            CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RxSTBC_CAPABLE);

        if(pHtCapability->ExtHtCapInfo.PlusHTC)
            CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_HTC_CAPABLE);

        if(pAd->CommonCfg.bRdg
                && pHtCapability->ExtHtCapInfo.RDGSupport)
            CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE);

        if(pHtCapability->ExtHtCapInfo.MCSFeedback == 0x03)
            CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_MCSFEEDBACK_CAPABLE);

        NdisMoveMemory(&pEntry->HTCapability, pHtCapability, HtCapabilityLen);

        assoc_ht_info_debugshow(pAd, pEntry, HtCapabilityLen, pHtCapability);
#ifdef DOT11_VHT_AC

        if(WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
                ie_list->vht_cap_len && ie_list->vht_op_len)
        {
            vht_mode_adjust(pAd, pEntry, &ie_list->vht_cap, &ie_list->vht_op);
            assoc_vht_info_debugshow(pAd, pEntry, &ie_list->vht_cap, &ie_list->vht_op);
        }

#endif /* DOT11_VHT_AC */
    }
    else
    {
        pAd->MacTab.fAnyStationIsLegacy = TRUE;
    }

#endif /* DOT11_N_SUPPORT */

    pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;
    pEntry->CurrTxRate = pEntry->MaxSupportedRate;

#ifdef MFB_SUPPORT
    pEntry->lastLegalMfb = 0;
    pEntry->isMfbChanged = FALSE;
    pEntry->fLastChangeAccordingMfb = FALSE;

    pEntry->toTxMrq = TRUE;
    pEntry->msiToTx = 0; /* has to increment whenever a mrq is sent */
    pEntry->mrqCnt = 0;

    pEntry->pendingMfsi = 0;

    pEntry->toTxMfb = FALSE;
    pEntry->mfbToTx = 0;
    pEntry->mfb0 = 0;
    pEntry->mfb1 = 0;
#endif /* MFB_SUPPORT */

    pEntry->freqOffsetValid = FALSE;

#ifdef TXBF_SUPPORT
    TxBFInit(pAd, pEntry, supportsETxBf);

    RTMPInitTimer(pAd, &pEntry->eTxBfProbeTimer, GET_TIMER_FUNCTION(eTxBfProbeTimerExec), pEntry, FALSE);
    NdisAllocateSpinLock(pAd, &pEntry->TxSndgLock);
#endif /* TXBF_SUPPORT */

    MlmeRAInit(pAd, pEntry);

    /* Set asic auto fall back */
    if(pAd->StaCfg.bAutoTxRateSwitch == TRUE)
    {
        UCHAR TableSize = 0;

        MlmeSelectTxRateTable(pAd, pEntry, &pEntry->pTable, &TableSize, &pEntry->CurrTxRateIndex);
        pEntry->bAutoTxRateSwitch = TRUE;
    }
    else
    {
        pEntry->HTPhyMode.field.MODE = pAd->StaCfg.HTPhyMode.field.MODE;
        pEntry->HTPhyMode.field.MCS = pAd->StaCfg.HTPhyMode.field.MCS;
        pEntry->bAutoTxRateSwitch = FALSE;

        /* If the legacy mode is set, overwrite the transmit setting of this entry. */
        RTMPUpdateLegacyTxSetting((UCHAR)pAd->StaCfg.DesiredTransmitSetting.field.FixedTxMode, pEntry);
    }


    pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
    pEntry->Sst = SST_ASSOC;
    pEntry->AuthState = AS_AUTH_OPEN;
    pEntry->AuthMode = pAd->StaCfg.AuthMode;
    pEntry->WepStatus = pAd->StaCfg.WepStatus;

    if(pEntry->AuthMode < Ndis802_11AuthModeWPA)
    {
        pEntry->WpaState = AS_NOTUSE;
        pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
    }
    else
    {
        pEntry->WpaState = AS_INITPSK;
        pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
    }

    if(pAd->StaCfg.BssType == BSS_INFRA)
    {
        UCHAR HashIdx = 0;
        MAC_TABLE_ENTRY *pCurrEntry = NULL;
        HashIdx = MAC_ADDR_HASH_INDEX(pAd->MlmeAux.Bssid);

        if(pAd->MacTab.Hash[HashIdx] == NULL)
        {
            pAd->MacTab.Hash[HashIdx] = pEntry;
        }
        else
        {
            pCurrEntry = pAd->MacTab.Hash[HashIdx];

            while(pCurrEntry->pNext != NULL)
            {
                pCurrEntry = pCurrEntry->pNext;
            }

            pCurrEntry->pNext = pEntry;
        }

        RTMPMoveMemory(pEntry->Addr, pAd->MlmeAux.Bssid, MAC_ADDR_LEN);
        pEntry->Aid = BSSID_WCID;
        pEntry->pAd = pAd;
        SET_ENTRY_CLIENT(pEntry);
        pAd->MacTab.Size ++;
    }

    NdisReleaseSpinLock(&pAd->MacTabLock);

#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT

    DBGPRINT(RT_DEBUG_INFO,
             ("ASSOC - Send assoc IEs to WpaSupplicant(0)\n"));


    if(pAd->StaCfg.WpaSupplicantUP)
    {

        DBGPRINT(RT_DEBUG_INFO,
                 ("ASSOC - Send assoc IEs to WpaSupplicant(1)\n"));

        SendAssocIEsToWpaSupplicant(pAd->net_dev, pAd->StaCfg.ReqVarIEs,
                                    pAd->StaCfg.ReqVarIELen);


        RtmpOSWirelessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM,
                                RT_ASSOC_EVENT_FLAG, NULL, NULL, 0);
    }

#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT

    DBGPRINT(RT_DEBUG_TRACE, ("Native wpa_supplicant support enabled (ReqVarIELen%d)\n", pAd->StaCfg.ReqVarIELen));

    {
        /*        union iwreq_data    wrqu; */
        wext_notify_event_assoc(pAd->net_dev, pAd->StaCfg.ReqVarIEs,
                                pAd->StaCfg.ReqVarIELen);


        RtmpOSWirelessEventSend(pAd->net_dev, RT_WLAN_EVENT_CGIWAP, -1,
                                pAd->MlmeAux.Bssid, NULL, 0);
    }
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */


    return TRUE;
}
