#ifndef NEO_PLAYER_H
#define NEO_PLAYER_H
#ifdef _WIN32
#pragma once
#endif

class CNEO_Player;

#include "basemultiplayerplayer.h"
#include "simtimer.h"
#include "soundenvelope.h"
#include "utldict.h"
#include "hl2mp_player.h"
#include "in_buttons.h"

#include "neo_player_shared.h"

class CNEO_Player : public CHL2MP_Player
{
public:
	DECLARE_CLASS(CNEO_Player, CHL2MP_Player);

	CNEO_Player();
	virtual ~CNEO_Player(void);

	static CNEO_Player *CreatePlayer(const char *className, edict_t *ed)
	{
		CNEO_Player::s_PlayerEdict = ed;
		return (CNEO_Player*)CreateEntityByName(className);
	}

	void SendTestMessage(const char *message);
	void SetTestMessageVisible(bool visible);

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache(void);
	virtual void Spawn(void);
	virtual void PostThink(void);
	virtual void PreThink(void);
	virtual void PlayerDeathThink(void);
	virtual void SetAnimation(PLAYER_ANIM playerAnim);
	virtual bool HandleCommand_JoinTeam(int team);
	virtual bool ClientCommand(const CCommand &args);
	virtual void CreateViewModel(int viewmodelindex = 0);
	virtual bool BecomeRagdollOnClient(const Vector &force);
	virtual void Event_Killed(const CTakeDamageInfo &info);
	virtual int OnTakeDamage(const CTakeDamageInfo &inputInfo);
	virtual bool WantsLagCompensationOnEntity(const CBasePlayer *pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits) const;
	virtual void FireBullets(const FireBulletsInfo_t &info);
	virtual bool Weapon_Switch(CBaseCombatWeapon *pWeapon, int viewmodelindex = 0);
	virtual bool BumpWeapon(CBaseCombatWeapon *pWeapon);
	virtual void ChangeTeam(int iTeam);
	virtual void PickupObject(CBaseEntity *pObject, bool bLimitMassAndSize);
	virtual void PlayStepSound(Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force);
	virtual void Weapon_Drop(CBaseCombatWeapon *pWeapon, const Vector *pvecTarget = NULL, const Vector *pVelocity = NULL);
	virtual void UpdateOnRemove(void);
	virtual void DeathSound(const CTakeDamageInfo &info);
	virtual CBaseEntity* EntSelectSpawnPoint(void);
	virtual void EquipSuit(bool bPlayEffects = true);
	virtual void RemoveSuit(void);
	virtual void GiveDefaultItems(void);

	virtual const Vector GetPlayerMaxs(void) const;
	virtual void InitVCollision(const Vector& vecAbsOrigin, const Vector& vecAbsVelocity);

	// Implementing in header in hopes of compiler picking up the inlined base method
	virtual float GetModelScale() const
	{
		switch (GetClass())
		{
		case NEO_CLASS_RECON:
			return CBaseAnimating::GetModelScale() * NEO_RECON_MODEL_SCALE;
		case NEO_CLASS_SUPPORT:
			return CBaseAnimating::GetModelScale() * NEO_SUPPORT_MODEL_SCALE;
		default:
			return CBaseAnimating::GetModelScale() * NEO_ASSAULT_MODEL_SCALE;
		}
	}
	
	void GiveLoadoutWeapon(void);

	void SetPlayerTeamModel(void);
	virtual void PickDefaultSpawnTeam(void);

	virtual bool StartObserverMode(int mode);
	virtual void StopObserverMode(void);

	virtual bool	CanHearAndReadChatFrom(CBasePlayer *pPlayer);

	inline bool IsCarryingGhost(void);
	inline bool IsAllowedToDrop(CBaseCombatWeapon *pWep);

	inline void ZeroFriendlyPlayerLocArray(void);

	void UpdateNetworkedFriendlyLocations(void);

	void Weapon_AimToggle(CBaseCombatWeapon *pWep);
	void Weapon_AimToggle(CNEOBaseCombatWeapon* pWep);

	void Lean(void);
	void SoftSuicide(void);
	void GiveAllItems(void);
	bool ProcessTeamSwitchRequest(int iTeam);

	void Weapon_SetZoom(const bool bZoomIn);

	void SuperJump(void);

	void RequestSetClass(int newClass);
	void RequestSetSkin(int newSkin);
	bool RequestSetLoadout(int loadoutNumber);

	int GetSkin() const { return m_iNeoSkin; }
	int GetClass() const { return m_iNeoClass; }

	bool IsAirborne() const { return (!(GetFlags() & FL_ONGROUND)); }

	virtual void StartAutoSprint(void);
	virtual void StartSprinting(void);
	virtual void StopSprinting(void);
	virtual void InitSprinting(void);
	virtual bool CanSprint(void);
	virtual void EnableSprint(bool bEnable);

	virtual void StartWalking(void);
	virtual void StopWalking(void);

	float GetNormSpeed_WithActiveWepEncumberment(void) const;
	float GetCrouchSpeed_WithActiveWepEncumberment(void) const;
	float GetWalkSpeed_WithActiveWepEncumberment(void) const;
	float GetSprintSpeed_WithActiveWepEncumberment(void) const;
	float GetNormSpeed_WithWepEncumberment(CNEOBaseCombatWeapon* pNeoWep) const;
	float GetCrouchSpeed_WithWepEncumberment(CNEOBaseCombatWeapon* pNeoWep) const;
	float GetWalkSpeed_WithWepEncumberment(CNEOBaseCombatWeapon* pNeoWep) const;
	float GetSprintSpeed_WithWepEncumberment(CNEOBaseCombatWeapon* pNeoWep) const;
	float GetNormSpeed(void) const;
	float GetCrouchSpeed(void) const;
	float GetWalkSpeed(void) const;
	float GetSprintSpeed(void) const;

	IMPLEMENT_NETWORK_VAR_FOR_DERIVED(m_EyeAngleOffset);

private:
	float GetActiveWeaponSpeedScale() const;
	float GetBackwardsMovementPenaltyScale() const { return ((m_nButtons & IN_BACK) ? NEO_SLOW_MODIFIER : 1.0); }

private:
	void CheckThermOpticButtons();
	void CheckVisionButtons();
	void PlayCloakSound();
	void CloakFlash();

	bool IsAllowedToSuperJump(void);

public:
	CNetworkVar(int, m_iNeoClass);
	CNetworkVar(int, m_iNeoSkin);

	CNetworkVar(int, m_iCapTeam);

	CNetworkVar(int, m_iXP);

	CNetworkVar(int, m_iLoadoutWepChoice);
	CNetworkVar(int, m_iNextSpawnClassChoice);

	CNetworkVar(bool, m_bShowTestMessage);
	CNetworkString(m_pszTestMessage, 32 * 2 + 1);

	CNetworkVector(m_vecGhostMarkerPos);
	CNetworkVar(int, m_iGhosterTeam);
	CNetworkVar(bool, m_bGhostExists);
	CNetworkVar(bool, m_bInThermOpticCamo);
	CNetworkVar(bool, m_bInVision);
	CNetworkVar(bool, m_bHasBeenAirborneForTooLongToSuperJump);
	CNetworkVar(bool, m_bInAim);

	CNetworkArray(Vector, m_rvFriendlyPlayerPositions, MAX_PLAYERS);

private:
	bool m_bInLeanLeft, m_bInLeanRight;
	bool m_bFirstDeathTick;
	bool m_bPreviouslyReloading;

	float m_flCamoAuxLastTime;
	float m_flLastAirborneJumpOkTime;
	float m_flLastSuperJumpTime;
};

inline CNEO_Player *ToNEOPlayer(CBaseEntity *pEntity)
{
	if (!pEntity || !pEntity->IsPlayer())
	{
		return NULL;
	}

	return dynamic_cast<CNEO_Player*>(pEntity);
}

#endif // NEO_PLAYER_H