library(ggplot2)
library(ggthemes)
library(RColorBrewer)
library(reshape2)
library(viridis)
library(Cairo)

sharkfins <- function(fname, title="") {
  data.orig <- read.csv(fname, comment.char='#')
  data.fins <- melt(data.orig, id=c("time"))
  data <- data.fins[seq(1, nrow(data.fins), 182), ]

  #mypalette <- colorRampPalette(c('red', 'blue', 'green'))
  num.clusters <- nlevels(factor(data$variable))
  #colors <- mypalette(num.clusters)
  colors <- viridis_pal(option="plasma")(num.clusters)
  colors.scramble <- sample(colors)
  sc <- scale_colour_manual(values=colors.scramble)

  fins <- ggplot(data,
                 aes(x=time,
                     y=value,
                     color=factor(variable))) +
                 geom_line(alpha=0.5) +
                 ggtitle(title) +
                 guides(colour=F) +
                 labs(x="Time Step", y="Agents Harvesting Cluster") +
                 stat_smooth(span=0.1) +
                 #theme_hc() +
                 #scale_colour_brewer(type="div", palette="BrBG")
                 sc
                 #theme_hc(bgcolor="darkunica") +
                 #scale_colour_hc("darkunica") +
                 #facet_wrap( ~ variable, ncol=num.clusters/4)

  return(fins)
}

position.map <- function(fname, title="", point=F, time.start=0, time.len=0) {
  data.orig <- read.csv(fname, comment.char='#')
  #data <- data.orig[seq(1, nrow(data.orig), 182), ]
  nr <- 0.25
  #data <- subset(data.orig, x >= nr | y >= nr | x <= -1*nr | y <= -1*nr)

  #data <- subset(data.orig, time >= 160000)

  data <- subset(data.orig, time >= time.start)
  if (time.len > 0) {
    data <- subset(data, time < (time.start + time.len))
  }

  if (!point) {
    pmap <- ggplot(data,
                   aes(x=x,
                       y=y)) +
                   stat_bin2d(bins=200, aes(alpha=..count..)) +
                   #ggtitle(title) +
                   guides(color=F, alpha=F, fill=F) +
                   #labs(x="X Coordinate", y="Y Coordinate") +
                   #labs(x="", y="") +
                   xlim(-15, 15) +
                   ylim(-15, 15) +
                   #theme_fivethirtyeight() +
                   #theme_few() +
                   theme(axis.text.x=element_blank(),
                         axis.text.y=element_blank(),
                         axis.ticks=element_blank(),
                         axis.title=element_blank(),
                         panel.background=element_blank(),
                         panel.border=element_rect(color="light gray", fill=NA),
                         plot.margin=margin(0.1,0.1,0,0, unit="cm")) +
                   scale_fill_viridis(name="Target Count", option="plasma") +
                   scale_alpha(range=c(0,1), limits=c(0,50))
  } else {
    pmap <- ggplot(data,
                   aes(x=x,
                       y=y,
                       col="Foraging Target")) +
                   geom_point(alpha=0.2) +
                   ggtitle(title) +
                   guides(color=F, alpha=F, fill=F) +
                   labs(x="X Coordinate", y="Y Coordinate") +
                   xlim(-15, 15) +
                   ylim(-15, 15) +
                   theme_fivethirtyeight() +
                   scale_colour_economist()
  }


  return(pmap)
}



# pdf("cpfa_cluster_sharkfins_regen_volatile.pdf", width=7, height=3)
# fins
# dev.off()
