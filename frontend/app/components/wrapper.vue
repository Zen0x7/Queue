<template>
  <div class="relative m-2 p-2 border border-gray-900/20 rounded-sm">
    <label class="absolute left-2 -top-2 bg-white px-2 text-xs font-light text-gray-900/40">
      {{ props.name }}
    </label>
    <slot/>
  </div>
</template>

<script lang="ts" setup>
const props = defineProps(['name'])
</script>