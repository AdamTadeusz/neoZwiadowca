#include "cbase.h"
#include "neo_gamerules.h"
#include "in_buttons.h"
#include "ammodef.h"

#ifdef CLIENT_DLL
	#include "c_neo_player.h"
#else
	#include "neo_player.h"
	#include "team.h"
	#include "neo_model_manager.h"
#endif

// NEO TODO (Rain): use GetPlayerSpawnSpot

REGISTER_GAMERULES_CLASS( CNEORules );

BEGIN_NETWORK_TABLE_NOBASE( CNEORules, DT_NEORules )
// NEO TODO (Rain): NEO specific game modes var (CTG/TDM/...)
#ifdef CLIENT_DLL
	//RecvPropInt( RECVINFO( m_iGameMode ) ),
#else
	//SendPropInt( SENDINFO( m_iGameMode ) ),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( neo_gamerules, CNEOGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( NEOGameRulesProxy, DT_NEOGameRulesProxy );

// NEO TODO (Rain): set accurately
static NEOViewVectors g_NEOViewVectors(
	Vector( 0, 0, 64 ),	   //VEC_VIEW (m_vView) 
							  
	Vector(-16, -16, 0 ),	  //VEC_HULL_MIN (m_vHullMin)
	Vector( 16,  16,  72 ),	  //VEC_HULL_MAX (m_vHullMax)
							  					
	Vector(-16, -16, 0 ),	  //VEC_DUCK_HULL_MIN (m_vDuckHullMin)
	Vector( 16,  16,  36 ),	  //VEC_DUCK_HULL_MAX	(m_vDuckHullMax)
	Vector( 0, 0, 28 ),		  //VEC_DUCK_VIEW		(m_vDuckView)
							  					
	Vector(-10, -10, -10 ),	  //VEC_OBS_HULL_MIN	(m_vObsHullMin)
	Vector( 10,  10,  10 ),	  //VEC_OBS_HULL_MAX	(m_vObsHullMax)
							  					
	Vector( 0, 0, 14 ),		  //VEC_DEAD_VIEWHEIGHT (m_vDeadViewHeight)

	Vector(-16, -16, 0 ),	  //VEC_CROUCH_TRACE_MIN (m_vCrouchTraceMin)
	Vector( 16,  16,  60 )	  //VEC_CROUCH_TRACE_MAX (m_vCrouchTraceMax)
);

#ifdef CLIENT_DLL
	void RecvProxy_NEORules( const RecvProp *pProp, void **pOut,
		void *pData, int objectID )
	{
		CNEORules *pRules = NEORules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CNEOGameRulesProxy, DT_NEOGameRulesProxy )
		RecvPropDataTable( "neo_gamerules_data", 0, 0,
			&REFERENCE_RECV_TABLE( DT_NEORules ),
			RecvProxy_NEORules )
	END_RECV_TABLE()
#else
	void *SendProxy_NEORules( const SendProp *pProp,
		const void *pStructBase, const void *pData,
		CSendProxyRecipients *pRecipients, int objectID )
	{
		CNEORules *pRules = NEORules();
		Assert( pRules );
		return pRules;
	}

	BEGIN_SEND_TABLE( CNEOGameRulesProxy, DT_NEOGameRulesProxy )
		SendPropDataTable( "neo_gamerules_data", 0,
			&REFERENCE_SEND_TABLE( DT_NEORules ),
			SendProxy_NEORules )
	END_SEND_TABLE()
#endif

// NEO NOTE (Rain): These are copied over from hl2mp gamerules implementation.
//
// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION			3.5
// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)

extern CBaseEntity *g_pLastJinraiSpawn, *g_pLastNSFSpawn;

CNEORules::CNEORules()
{
#ifndef CLIENT_DLL
	DevMsg("CNEORules serverside ctor\n");
	
	Q_strncpy(g_Teams[TEAM_JINRAI]->m_szTeamname.GetForModify(),
		TEAM_STR_JINRAI, MAX_TEAM_NAME_LENGTH);
	
	Q_strncpy(g_Teams[TEAM_NSF]->m_szTeamname.GetForModify(),
		TEAM_STR_NSF, MAX_TEAM_NAME_LENGTH);
	
	Msg("Server teams %s & %s\n",
		g_Teams[TEAM_JINRAI]->GetName(), g_Teams[TEAM_NSF]->GetName());
	
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *player = UTIL_PlayerByIndex(i);
		if (player)
		{
			g_Teams[TEAM_JINRAI]->UpdateClientData(player);
			g_Teams[TEAM_NSF]->UpdateClientData(player);
		}
	}
#endif
}

CNEORules::~CNEORules()
{

}

// This is the HL2MP gamerules GetAmmoDef() global scope function copied over,
// because we want to implement it ourselves. This can be refactored out if/when
// we don't want to support HL2MP guns anymore.
CAmmoDef *GetAmmoDef_HL2MP()
{
	static CAmmoDef def;
	static bool bInitted = false;

	if (!bInitted)
	{
		bInitted = true;

		def.AddAmmoType("AR2", DMG_BULLET, TRACER_LINE_AND_WHIZ, 0, 0, 60, BULLET_IMPULSE(200, 1225), 0);
		def.AddAmmoType("AR2AltFire", DMG_DISSOLVE, TRACER_NONE, 0, 0, 3, 0, 0);
		def.AddAmmoType("Pistol", DMG_BULLET, TRACER_LINE_AND_WHIZ, 0, 0, 150, BULLET_IMPULSE(200, 1225), 0);
		def.AddAmmoType("SMG1", DMG_BULLET, TRACER_LINE_AND_WHIZ, 0, 0, 225, BULLET_IMPULSE(200, 1225), 0);
		def.AddAmmoType("357", DMG_BULLET, TRACER_LINE_AND_WHIZ, 0, 0, 12, BULLET_IMPULSE(800, 5000), 0);
		def.AddAmmoType("XBowBolt", DMG_BULLET, TRACER_LINE, 0, 0, 10, BULLET_IMPULSE(800, 8000), 0);
		def.AddAmmoType("Buckshot", DMG_BULLET | DMG_BUCKSHOT, TRACER_LINE, 0, 0, 30, BULLET_IMPULSE(400, 1200), 0);
		def.AddAmmoType("RPG_Round", DMG_BURN, TRACER_NONE, 0, 0, 3, 0, 0);
		def.AddAmmoType("SMG1_Grenade", DMG_BURN, TRACER_NONE, 0, 0, 3, 0, 0);
		def.AddAmmoType("Grenade", DMG_BURN, TRACER_NONE, 0, 0, 5, 0, 0);
		def.AddAmmoType("slam", DMG_BURN, TRACER_NONE, 0, 0, 5, 0, 0);
	}

	return &def;
}

CAmmoDef *GetAmmoDef()
{
	static CAmmoDef *def;
	static bool bInitted = false;

	if (!bInitted)
	{
		bInitted = true;

		// HL2MP ammo support
		def = GetAmmoDef_HL2MP();

		// NEO ammo support
		def->AddAmmoType("AMMO_10G_SHELL", DMG_BULLET | DMG_BUCKSHOT,
			TRACER_LINE, 0, 0, 30, BULLET_IMPULSE(400, 1200), 0);
	}

	return def;
}

bool CNEORules::ShouldCollide(int collisionGroup0, int collisionGroup1)
{
	return BaseClass::ShouldCollide(collisionGroup0, collisionGroup1);
}

void CNEORules::Think(void)
{
	BaseClass::Think();
}

void CNEORules::CreateStandardEntities(void)
{
	BaseClass::CreateStandardEntities();

#ifdef GAME_DLL
	g_pLastJinraiSpawn = NULL;
	g_pLastNSFSpawn = NULL;
#endif
}

int CNEORules::WeaponShouldRespawn(CBaseCombatWeapon *pWep)
{
	return BaseClass::WeaponShouldRespawn(pWep);
}

const char *CNEORules::GetGameDescription(void)
{
	//DevMsg("Querying CNEORules game description\n");
	return BaseClass::GetGameDescription();
}

const CViewVectors *CNEORules::GetViewVectors() const
{
	return &g_NEOViewVectors;
}

const NEOViewVectors* CNEORules::GetNEOViewVectors() const
{
	return &g_NEOViewVectors;
}

float CNEORules::GetMapRemainingTime()
{
	return BaseClass::GetMapRemainingTime();
}

#ifndef CLIENT_DLL
void CNEORules::CleanUpMap()
{
	BaseClass::CleanUpMap();
}

void CNEORules::CheckRestartGame()
{
	BaseClass::CheckRestartGame();
}

void CNEORules::RestartGame()
{
	BaseClass::RestartGame();
}
#endif

#ifdef GAME_DLL
bool CNEORules::ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen)
{
	return BaseClass::ClientConnected(pEntity, pszName, pszAddress,
		reject, maxrejectlen);
#if(0)
	const bool canJoin = BaseClass::ClientConnected(pEntity, pszName, pszAddress,
		reject, maxrejectlen);

	if (canJoin)
	{
	}

	return canJoin;
#endif
}
#endif

void CNEORules::ClientSettingsChanged(CBasePlayer *pPlayer)
{
#ifndef CLIENT_DLL
	CNEO_Player *pNEOPlayer = ToNEOPlayer(pPlayer);

	if (!pNEOPlayer)
	{
		return;
	}

	CNEOModelManager *mm = CNEOModelManager::Instance();

	const char *pCurrentModel = modelinfo->GetModelName(pNEOPlayer->GetModel());
	const char *pTargetModel = mm->GetPlayerModel(
		(NeoSkin)pNEOPlayer->m_nNeoSkin.Get(),
		(NeoClass)pNEOPlayer->m_nCyborgClass.Get(),
		pNEOPlayer->GetTeamNumber());

	if (V_stricmp(pCurrentModel, pTargetModel))
	{
		pNEOPlayer->SetPlayerTeamModel();
	}

	// We're skipping calling the base CHL2MPRules method here
	CTeamplayRules::ClientSettingsChanged(pPlayer);
#endif
}