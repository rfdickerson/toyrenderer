ktx create --encode uastc --zstd 18 --format R8G8B8_SRGB --assign-oetf srgb --assign-primaries bt709 --cubemap --generate-mipmap posx.jpg negx.jpg posy.jpg negy.jpg posz.jpg negz.jpg pond.ktx2


ktx create --encode uastc --zstd 18 --format R8G8B8_SRGB --assign-oetf srgb --assign-primaries bt709 --cubemap --generate-mipmap px.png nx.png py.png ny.png pz.png nz.png rosendal.ktx2

ktx create --encode uastc --zstd 18 --format R8G8B8_SRGB --assign-oetf srgb --assign-primaries bt709 --generate-mipmap oldtruck_d.png oldtruck_d.ktx2

ktx create --encode uastc --zstd 18 --format R8G8_UNORM --assign-oetf linear --assign-primaries none --generate-mipmap oldtruck_normal.png oldtruck_n.ktx2


// cobblestone

ktx create --encode uastc --zstd 18 --format R8G8B8A8_SRGB --assign-oetf srgb --assign-primaries bt709 --generate-mipmap cobblestone_d.png cobblestone_d.ktx2

ktx create --encode uastc --zstd 18 --format R8G8_UNORM --normal-mode --generate-mipmap cobblestone_n.png cobblestone_n.ktx2