#include "SceneBase.h"

#include "GameMap.h"
#include "GameMapDef.h"
#include "MapManager.h"
#include "MapCfgMgr.h"
#include "NavScene.h"
#include "NavSceneMgr.h"
#include "NavSceneQuery.h"
#include "PhysicMgr.h"
#include "PhysicScene.h"
#include "SceneCfg.h"
#include "SceneObject.h"
#include "SceneObjectAOI.h"
#include "SceneTree.h"
#include "SceneSetMgr.h"


CSceneBase::CSceneBase() {}

CSceneBase::~CSceneBase() {}

bool CSceneBase::init(const SceneIdx& idxScene, CSceneBaseSetMgr* pSceneSetMgr, CMapManager* pMapManager)
{
    __ENTER_FUNCTION
    m_idxScene  = idxScene;
    m_pSceneSetMgr = pSceneSetMgr;
    m_pSceneCfg = pMapManager->getMapCfgMgr()->querySceneCfg(idxScene.getSceneResID());
    CHECKF_FMT(m_pSceneCfg, "querySceneCfg Fail scene:{}", idxScene.getSceneResID());
    m_pMap = pMapManager->queryMap(m_pSceneCfg->getMapID());
    CHECKF_FMT(m_pMap, "FIND Map Fail:{} scene:{}", m_pSceneCfg->getMapID(), m_idxScene);

    return true;
    __LEAVE_FUNCTION
    return false;
}

bool CSceneBase::initSceneTree(const CPos3D& vBasePos, double fWidth, double fHeight, uint32_t nTileGridRange, bool bDynamicSetLev)
{
    __ENTER_FUNCTION
    if(m_pSceneTree)
    {
        return false;
    }
    CSceneTree* pSceneTree = CreateNewPtr<CSceneTree>(m_pMap, getID(), vBasePos, fWidth, fHeight, nTileGridRange, bDynamicSetLev);
    CHECKF(pSceneTree);
    m_pSceneTree.reset(pSceneTree);

    //physic
    if(hasMapFlag(MAP_FLAG::MAPFLAG_DYNA_PHYSX))
    {
        if(m_pMap->getPhysicSceneFileName().empty() == false)
        {
            auto mgr = m_pSceneSetMgr->getPhysicMgr();
            CHECKF(mgr);
            m_pDynamicPhysicScene = mgr->createDynamicScene(m_pMap->getPhysicSceneFileName(), vBasePos, fWidth, fHeight);
            CHECKF(m_pDynamicPhysicScene);
        }
    }
    else
    {
        if(m_pMap->getPhysicSceneFileName().empty() == false)
        {
            auto mgr = m_pSceneSetMgr->getPhysicMgr();
            CHECKF(mgr);
            m_pPhysicScene = mgr->createStaticScene(m_pMap->getPhysicSceneFileName());
        }
    }


    //navmesh
    if(hasMapFlag(MAP_FLAG::MAPFLAG_DYNA_NAV))
    {
        if(m_pMap->getNavSceneFileName().empty() == false)
        {
            auto mgr = m_pSceneSetMgr->getNavMeshMgr();
            CHECKF(mgr);
            auto full_filename = fmt::format("res/navmesh/{}", m_pMap->getNavSceneFileName());
            m_pDynamicNavScene = mgr->createDynamicScene(full_filename, vBasePos, fWidth, fHeight);
            CHECKF(m_pDynamicNavScene);
        }
    }
    else
    {
        
        if(m_pMap->getNavSceneFileName().empty() == false)
        {
            auto mgr = m_pSceneSetMgr->getNavMeshMgr();
            CHECKF(mgr);
            auto full_filename = fmt::format("res/navmesh/{}", m_pMap->getNavSceneFileName());
            m_pNavScene = mgr->createStaticScene(full_filename);
        }
    }



    
    LOGDEBUG("CSceneBase::initSceneTree {} {:p} Tree:{:p}", getSceneIdx(), (void*)this, (void*)pSceneTree);
    return true;
    __LEAVE_FUNCTION
    return false;
}

bool CSceneBase::linkSceneTree(CSceneBase* pLinkScene)
{
    __ENTER_FUNCTION
    if(m_pSceneTree)
    {
        return false;
    }
    CHECKF(pLinkScene->getMap()->getMapID() == getMapResID());

    m_pSceneTree   = pLinkScene->m_pSceneTree;
    m_pDynamicPhysicScene = pLinkScene->m_pDynamicPhysicScene;
    m_pDynamicNavScene    = pLinkScene->m_pDynamicNavScene;
    LOGDEBUG("CSceneBase::initSceneTree {} {:p} Tree:{:p}", getSceneIdx(), (void*)this, (void*)m_pSceneTree.get());
    return true;
    __LEAVE_FUNCTION
    return false;
}

uint64_t CSceneBase::getScriptID() const
{
    return m_pSceneCfg->getScriptID();
}

uint32_t CSceneBase::getMapResID() const
{
    return m_pMap->getMapID();
}

uint16_t CSceneBase::getSceneResID() const
{
    return m_pSceneCfg->getSceneResID();
}

const CPhysicScene* CSceneBase::getPhysicScene() const
{
    if(m_pDynamicPhysicScene)
        return m_pDynamicPhysicScene.get();
    return m_pPhysicScene;
}

const CNavScene* CSceneBase::getNavScene() const
{
    if(m_pDynamicNavScene)
        return m_pDynamicNavScene.get();
    return m_pNavScene;
}

CPhysicScene* CSceneBase::getDynamicPhysicSceneRef() const
{
    return m_pDynamicPhysicScene.get();
}

CNavScene* CSceneBase::getDynamicNavSceneRef() const
{
    return m_pDynamicNavScene.get();
}

CSceneObject* CSceneBase::queryPlayerObj(uint64_t idObj) const
{
    __ENTER_FUNCTION
    auto it = m_setPlayer.find(idObj);
    if(it != m_setPlayer.end())
    {
        return it->second;
    }
    __LEAVE_FUNCTION

    return nullptr;
}

CSceneObject* CSceneBase::querySceneObj(uint64_t idObj) const
{
    __ENTER_FUNCTION
    auto it = m_setActor.find(idObj);
    if(it != m_setActor.end())
    {
        return it->second;
    }
    __LEAVE_FUNCTION

    return nullptr;
}

void CSceneBase::foreachSceneObj(const std::function<void(CSceneObject*)>& func) const
{
    __ENTER_FUNCTION
    for(auto it = m_setActor.begin(); it != m_setActor.end(); ++it)
    {
        __ENTER_FUNCTION
        func(it->second);
        __LEAVE_FUNCTION
    }
    __LEAVE_FUNCTION
}

void CSceneBase::foreachPlayerObj(const std::function<void(CSceneObject*)>& func) const
{
    __ENTER_FUNCTION
    for(auto it = m_setPlayer.begin(); it != m_setPlayer.end(); ++it)
    {
        __ENTER_FUNCTION
        func(it->second);
        __LEAVE_FUNCTION
    }
    __LEAVE_FUNCTION
}

bool CSceneBase::enterMap(CSceneObject* pActor, const CPos3D& pos, float face)
{
    __ENTER_FUNCTION
    if(m_pSceneTree->isInsideScene(pos) == false)
    {
        LOGERROR("enterMap Pos Fail id:{} pos:{} scene:{} ", pActor->getID(), pos, getSceneIdx());
        LOGTRACE("CallStack: {}", get_stack_trace_string(1, 7));
        return false;
    }

    if(pActor->isPlayer())
        m_setPlayer[pActor->getID()] = pActor;
    m_setActor[pActor->getID()] = pActor;

    pActor->_setPos(pos);
    pActor->setFace(face);

    pActor->onEnterMap(this);
    pActor->getAOI()->initViewList();



    m_pSceneTree->checkNeedResizeSceneTile(m_setPlayer.size());
    return true;
    __LEAVE_FUNCTION
    return false;
}

void CSceneBase::leaveMap(CSceneObject* pActor, uint32_t idTargetScene /*= 0*/)
{
    _leaveMap(pActor, idTargetScene);
}

void CSceneBase::_leaveMap(CSceneObject* pActor, uint32_t idTargetScene /*= 0*/)
{
    __ENTER_FUNCTION
    if(pActor->isPlayer())
        m_setPlayer.erase(pActor->getID());

    m_setActor.erase(pActor->getID());

    //将玩家从场景树移除
    pActor->getAOI()->clearViewList(true);
    pActor->onLeaveMap(idTargetScene);

    m_pSceneTree->checkNeedResizeSceneTile(m_setPlayer.size());
    __LEAVE_FUNCTION
}

std::optional<CPos3D> CSceneBase::findStandPosNearby(const CPos3D& pos, float range) const
{
    __ENTER_FUNCTION
    CHECK_RET_FMT(m_pSceneTree->isInsideScene(pos), {}, "pos:{}", pos);
    auto nav_scene = getNavScene();
    CHECKF(nav_scene);
    auto nav_query = nav_scene->getQuery();
    CHECKF(nav_query);
    auto result = nav_query->findRandomPointAroundCircle(pos, range);
    if(result.has_value() == false)
    {
        auto result_pos = m_pSceneTree->clampPosInBound(pos);
        return result_pos;
    }
    
    auto result_pos = result.value();
    result_pos = m_pSceneTree->clampPosInBound(result_pos);

    return result_pos;
    __LEAVE_FUNCTION
    return {};
}

bool CSceneBase::isPassDisable(const CPos3D& spos, const CPos3D& epos, uint32_t actor_type) const
{
    __ENTER_FUNCTION
    CHECK_RET_FMT(m_pSceneTree->isInsideScene(spos), false, "spos:{}", spos);
    CHECK_RET_FMT(m_pSceneTree->isInsideScene(epos), false, "epos:{}", epos);
    auto physx_scene = getPhysicScene();
    CHECKF(physx_scene);
    auto result = physx_scene->sweep(spos, epos);
    if(result)
    {
        return false;
    }
    __LEAVE_FUNCTION
    return true;
}

bool CSceneBase::isPvPDisable(const CPos3D& pos) const
{
    __ENTER_FUNCTION
    if(hasMapFlag(MAP_FLAG::MAPFLAG_DISABLE_PK) == true)
        return true;

    return false;
    __LEAVE_FUNCTION
    return true;
}

bool CSceneBase::isPvPFree(const CPos3D& pos) const
{
    __ENTER_FUNCTION
    if(hasMapFlag(MAP_FLAG::MAPFLAG_PK_FREE) == true)
        return true;

    return false;
    __LEAVE_FUNCTION
    return true;
}

bool CSceneBase::isRecordDisable(const CPos3D& pos) const
{
    __ENTER_FUNCTION
    if(hasMapFlag(MAP_FLAG::MAPFLAG_DISABLE_RECORD) == true)
        return true;

    return false;
    __LEAVE_FUNCTION
    return true;
}

bool CSceneBase::isDropDisable(const CPos3D& pos) const
{
    __ENTER_FUNCTION
    if(hasMapFlag(MAP_FLAG::MAPFLAG_DISABLE_DROP) == true)
        return true;

    return false;
    __LEAVE_FUNCTION
    return true;
}

bool CSceneBase::isStallDisable(const CPos3D& pos) const
{
    __ENTER_FUNCTION
    if(hasMapFlag(MAP_FLAG::MAPFLAG_DISABLE_STALL) == true)
        return true;

    return false;
    __LEAVE_FUNCTION
    return true;
}

bool CSceneBase::hasMapFlagMask(const MapFlagMask& mask) const
{
    return m_pSceneCfg->hasMapFlagMask(mask);
}

bool CSceneBase::hasMapFlag(MAP_FLAG flag) const
{
    return m_pSceneCfg->hasMapFlag(flag);
}