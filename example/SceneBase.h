#ifndef SCENEBASE_H
#define SCENEBASE_H

#include "BaseCode.h"
#include "GameMapDef.h"
#include "SceneID.h"
class CSceneObject;
class CMapManager;
class CSceneTree;
class CGameMap;
class CPhysicScene;
class CNavScene;
class CSceneCfg;
class CSceneSet;
class CEventManager;
class CSceneBaseSetMgr;

export_lua enum SceneState
{
    SCENESTATE_CREATED      = 0,
    SCENESTATE_WAIT_LOADING = 1,
    SCENESTATE_NORMAL       = 2,
    SCENESTATE_WAIT_DESTROY = 3,
};

export_lua class CSceneBase : NoncopyableT<CSceneBase>
{
protected:
    CSceneBase();

public:
    CreateNewImpl(CSceneBase);
    virtual ~CSceneBase();

public:
    bool init(const SceneIdx& idxScene, CSceneBaseSetMgr* pSceneSetMgr, CMapManager* pMapManager);

    export_lua virtual void destroy() {}

    export_lua SceneState getSceneState() const { return m_curSceneState; }
    export_lua void setSceneState(SceneState val) { m_curSceneState = val; }

    export_lua virtual bool isStatic() const { return true; }

    export_lua uint64_t getID() const { return m_idxScene; }

    export_lua uint32_t getMapResID() const;
    export_lua uint16_t getSceneResID() const;
    export_lua uint64_t getScriptID() const;

    export_lua uint16_t getCellID() const { return getSceneIdx().getCellID(); }
    export_lua uint32_t getCellInstanceID() const { return getSceneIdx().getCellInstanceID(); }

    export_lua CSceneBaseSetMgr* getSceneSetMgr() const { return m_pSceneSetMgr; }
    export_lua const SceneIdx&   getSceneIdx() const { return m_idxScene; }

    export_lua uint64_t getSpecialID() const { return m_idSpecial; }
    export_lua void     setSpecialID(uint64_t id) { m_idSpecial = id; }

    export_lua const CGameMap*   getMap() const { return m_pMap; }
    export_lua const CSceneCfg*  getSceneCfg() const { return m_pSceneCfg; }

    export_lua CSceneTree* getSceneTree() const { return m_pSceneTree.get(); }
    
    //如果本场景不是动态物理场景，则返回nullptr
    export_lua CPhysicScene*       getDynamicPhysicSceneRef() const;
    export_lua CNavScene*          getDynamicNavSceneRef() const;
    //如果本场景是动态物理场景则返回自己的，否则返回公共场景的静态物理场景
    export_lua const CPhysicScene* getPhysicScene() const;
    export_lua const CNavScene*    getNavScene() const;

    virtual CEventManager* getEventMgr(uint32_t idCamp) const = 0;

    bool initSceneTree(const CPos3D& vBasePos, double fWidth, double fHeight, uint32_t nTileGridRange, bool bDynamicSetLev = false);
    bool linkSceneTree(CSceneBase* pLinkScene);

    virtual bool enterMap(CSceneObject* pActor, const CPos3D& pos, float face);
    virtual void leaveMap(CSceneObject* pActor, uint32_t idTargetScene = 0);
    void         _leaveMap(CSceneObject* pActor, uint32_t idTargetScene = 0);

    export_lua size_t getActorCount() const { return m_setActor.size(); }
    export_lua size_t getPlayerCount() const { return m_setPlayer.size(); }

    export_lua CSceneObject* querySceneObj(uint64_t idObj) const;
    export_lua CSceneObject* queryPlayerObj(uint64_t idObj) const;

    void foreachSceneObj(const std::function<void(CSceneObject*)>& func) const;
    void foreachPlayerObj(const std::function<void(CSceneObject*)>& func) const;


    export_lua virtual std::optional<CPos3D> findStandPosNearby(const CPos3D& pos, float range) const;

    export_lua virtual bool isPassDisable(const CPos3D& spos, const CPos3D& epos, uint32_t actor_type) const;
    export_lua virtual bool isPvPDisable(const CPos3D& pos) const;
    export_lua virtual bool isStallDisable(const CPos3D& pos) const;
    export_lua virtual bool isRecordDisable(const CPos3D& pos) const;
    export_lua virtual bool isDropDisable(const CPos3D& pos) const;
    export_lua virtual bool isPvPFree(const CPos3D& pos) const;
    export_lua virtual bool hasMapFlag(MAP_FLAG flag) const;
    virtual bool hasMapFlagMask(const MapFlagMask& mask) const;

protected:
    CSceneBaseSetMgr* m_pSceneSetMgr = nullptr;
    const CGameMap*  m_pMap      = nullptr;
    const CSceneCfg* m_pSceneCfg = nullptr;

    SceneState m_curSceneState = SCENESTATE_CREATED;

    SceneIdx m_idxScene;
    uint64_t m_idSpecial = 0;

    std::shared_ptr<CSceneTree>   m_pSceneTree;

    const CPhysicScene* m_pPhysicScene = nullptr;
    const CNavScene*    m_pNavScene    = nullptr;
    std::shared_ptr<CNavScene>    m_pDynamicNavScene;
    std::shared_ptr<CPhysicScene> m_pDynamicPhysicScene;

    std::unordered_map<uint64_t/* actor_id */, CSceneObject*> m_setPlayer;
    std::unordered_map<uint64_t/*actor_id*/, CSceneObject*> m_setActor;
};

#endif /* SCENEBASE_H */
