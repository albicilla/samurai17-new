<img src="data/logo.jpg" width=25% align="right" />

# for Samurai17
samuraiEnv + OpenAI Baselinesです。Baselines側もカスタマイズしているので、トップディレクトリで`pip install -e .`してください。
更新したくないpackageはsetup.pyからコメントアウトしてください。

gymEnv/Baselines/train.pyで学習します。save以下にモデルの一時保存が順次作られます。
Ubuntuだと/tmp以下に[Open-AI-日付ディレクトリ]を作って、Logを吐きます。
そのディレクトリでtensorboard --logdir tdするとtensorboardが立ち上がるので、ブラウザでlocalhost:6006にアクセスしてください。

gym環境もバクを直して同じ階層のgymEnv3になりました。こちらを使ってください。
human_play.pyは gymEnv/Baselines/human_play.pyから動かせます。

# Baselines

OpenAI Baselines is a set of high-quality implementations of reinforcement learning algorithms.

These algorithms will make it easier for the research community to replicate, refine, and identify new ideas, and will create good baselines to build research on top of. Our DQN implementation and its variants are roughly on par with the scores in published papers. We expect they will be used as a base around which new ideas can be added, and as a tool for comparing a new approach against existing ones. 

You can install it by typing:

```bash
git clone https://github.com/openai/baselines.git
cd baselines
pip install -e .
```

- [A2C](baselines/a2c)
- [ACER](baselines/acer)
- [ACKTR](baselines/acktr)
- [DDPG](baselines/ddpg)
- [DQN](baselines/deepq)
- [PPO1](baselines/ppo1) (Multi-CPU using MPI)
- [PPO2](baselines/ppo2) (Optimized for GPU)
- [TRPO](baselines/trpo_mpi)

To cite this repository in publications:

    @misc{baselines,
      author = {Dhariwal, Prafulla and Hesse, Christopher and Plappert, Matthias and Radford, Alec and Schulman, John and Sidor, Szymon and Wu, Yuhuai},
      title = {OpenAI Baselines},
      year = {2017},
      publisher = {GitHub},
      journal = {GitHub repository},
      howpublished = {\url{https://github.com/openai/baselines}},
    }
