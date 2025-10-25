using UnrealBuildTool;

public class MistsofKrakovia : ModuleRules
{
    public MistsofKrakovia(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Runtime comum
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core","CoreUObject","Engine","InputCore","ReplicatedInventory"
        });

        // Só no Client/Editor (Server não precisa de EnhancedInput/UMG)
        if (Target.Type != TargetType.Server)
        {
            PublicDependencyModuleNames.AddRange(new string[] {
                "EnhancedInput"
                // "UMG" // adiciona aqui quando usar HUD
            });
        }

        // Se for usar AI pros mobs (quando chegar lá):
        // PublicDependencyModuleNames.Add("AIModule");

        // NADA de UnrealEd/Editor aqui
    }
}
