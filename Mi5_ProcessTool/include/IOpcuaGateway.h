#ifndef IOPCUAGATEWAY_H
#define IOPCUAGATEWAY_H
//
//#include <ClusterInstance/Behavior/GoldenImageCalculationState.h>
//
///**
//*** Interface for a behavior to be adressed by the `BehaviorManager`.
//**/
//class IManageableClusterBehavior
//{
//public:
//    virtual int getToolSide() = 0;
//    virtual bool getBusy() = 0;
//    virtual GoldenImageCalculationState getGoldenImageCalculationState() = 0;
//    virtual void notifyGoldenImageCalculationStateChanged(GoldenImageCalculationState state) = 0;
//    virtual void notifyNewPreprocessedImage(int side, int slot) = 0;
//
//signals:
//    void registerNewImage(IManageableClusterBehavior*
//                          pSender);                 /** < Notify about the reception of a new defect scan image. **/
//    void lastImageOfCurrentBatchFinished(int side);
//    void requestGoldenImageState(IManageableClusterBehavior*
//                                 pSender);          /** < Trigger the request of the golden image calculation state. **/
//    void registerPrimaryCluser(IManageableClusterBehavior* pSender);
//    void deregisterPrimaryCluser(IManageableClusterBehavior* pSender);
//    void preprocessingFinished(int side, int slot);
//    void goldenImageCalculationStateChanged(int side,
//                                            GoldenImageCalculationState state);   // TODO: Primary behavior only
//};

#endif // IOPCUAGATEWAY_H