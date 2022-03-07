const { description } = require('../../package')

module.exports = {
  /**
   * Ref：https://v1.vuepress.vuejs.org/config/#title
   */
  title: 'Vuepress Docs Boilerplate',
  /**
   * Ref：https://v1.vuepress.vuejs.org/config/#description
   */
  description: description,

  base: '/iots-rf-gateway/',


  /**
   * Theme configuration, here is the default theme configuration for VuePress.
   *
   * ref：https://v1.vuepress.vuejs.org/theme/default-theme-config.html
   */

   themeConfig: {
    sidebarDepth: 3,
    sidebar: [
      '/'
    ]
  },
  markdown: {
		lineNumbers: true,
		extendMarkdown: md => {
		  md.set({ html: true })
		  md.use(require('markdown-it-plantuml'))
		}
	}
}
